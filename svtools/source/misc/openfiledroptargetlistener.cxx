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

#include <svtools/openfiledroptargetlistener.hxx>

#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <vcl/transfer.hxx>
#include <sot/filelist.hxx>

#include <osl/file.hxx>
#include <vcl/svapp.hxx>

OpenFileDropTargetListener::OpenFileDropTargetListener( const css::uno::Reference< css::uno::XComponentContext >& xContext,
                                        const css::uno::Reference< css::frame::XFrame >&          xFrame  )
        : m_xContext      ( xContext                      )
        , m_xTargetFrame  ( xFrame                        )
{
}


OpenFileDropTargetListener::~OpenFileDropTargetListener()
{
    m_xTargetFrame.clear();
    m_xContext.clear();
}


void SAL_CALL OpenFileDropTargetListener::disposing( const css::lang::EventObject& )
{
    m_xTargetFrame.clear();
    m_xContext.clear();
}


void SAL_CALL OpenFileDropTargetListener::drop( const css::datatransfer::dnd::DropTargetDropEvent& dtde )
{
    const sal_Int8 nAction = dtde.DropAction;

    try
    {
        if ( css::datatransfer::dnd::DNDConstants::ACTION_NONE != nAction )
        {
            TransferableDataHelper aHelper( dtde.Transferable );
            bool bFormatFound = false;
            FileList aFileList;

            // at first check filelist format
            if ( aHelper.GetFileList( SotClipboardFormatId::FILE_LIST, aFileList ) )
            {
                sal_uLong i, nCount = aFileList.Count();
                for ( i = 0; i < nCount; ++i )
                    implts_OpenFile( aFileList.GetFile(i) );
                bFormatFound = true;
            }

            // then, if necessary, the file format
            OUString aFilePath;
            if ( !bFormatFound && aHelper.GetString( SotClipboardFormatId::SIMPLE_FILE, aFilePath ) )
                implts_OpenFile( aFilePath );
        }
        dtde.Context->dropComplete( css::datatransfer::dnd::DNDConstants::ACTION_NONE != nAction );
    }
    catch( const css::uno::Exception& )
    {
    }
}


void SAL_CALL OpenFileDropTargetListener::dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee )
{
    try
    {
        implts_BeginDrag( dtdee.SupportedDataFlavors );
    }
    catch( const css::uno::Exception& )
    {
    }

    dragOver( dtdee );
}


void SAL_CALL OpenFileDropTargetListener::dragExit( const css::datatransfer::dnd::DropTargetEvent& )
{
    try
    {
        implts_EndDrag();
    }
    catch( const css::uno::Exception& )
    {
    }
}


void SAL_CALL OpenFileDropTargetListener::dragOver( const css::datatransfer::dnd::DropTargetDragEvent& dtde )
{
    try
    {
        bool bAccept = ( implts_IsDropFormatSupported( SotClipboardFormatId::SIMPLE_FILE ) ||
                             implts_IsDropFormatSupported( SotClipboardFormatId::FILE_LIST ) );

        if ( !bAccept )
            dtde.Context->rejectDrag();
        else
            dtde.Context->acceptDrag( css::datatransfer::dnd::DNDConstants::ACTION_COPY );
    }
    catch( const css::uno::Exception& )
    {
    }
}


void SAL_CALL OpenFileDropTargetListener::dropActionChanged( const css::datatransfer::dnd::DropTargetDragEvent& )
{
}

void OpenFileDropTargetListener::implts_BeginDrag( const css::uno::Sequence< css::datatransfer::DataFlavor >& rSupportedDataFlavors )
{
    /* SAFE { */
    SolarMutexGuard aGuard;

    m_aFormats.clear();
    TransferableDataHelper::FillDataFlavorExVector(rSupportedDataFlavors, m_aFormats);
    /* } SAFE */
}

void OpenFileDropTargetListener::implts_EndDrag()
{
    /* SAFE { */
    SolarMutexGuard aGuard;

    m_aFormats.clear();
    /* } SAFE */
}

bool OpenFileDropTargetListener::implts_IsDropFormatSupported( SotClipboardFormatId nFormat )
{
    /* SAFE { */
    SolarMutexGuard aGuard;

    for (auto const& format : m_aFormats)
    {
        if (nFormat == format.mnSotId)
        {
            return true;
        }
    }
    /* } SAFE */

    return false;
}

void OpenFileDropTargetListener::implts_OpenFile( const OUString& rFilePath )
{
    OUString aFileURL;
    if ( osl::FileBase::getFileURLFromSystemPath( rFilePath, aFileURL ) != osl::FileBase::E_None )
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
        // Create a new task or recycle an existing one
        css::uno::Reference< css::frame::XDispatch > xDispatcher = xProvider->queryDispatch( aURL, "_default", 0 );
        if ( xDispatcher.is() )
            xDispatcher->dispatch( aURL, css::uno::Sequence < css::beans::PropertyValue >() );
    }
    /* } SAFE */
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
