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

#include <osl/diagnose.h>
#include "WinClipbImpl.hxx"

#include <systools/win32/comtools.hxx>
#include "DtObjFactory.hxx"
#include "APNDataObject.hxx"
#include "DOTransferable.hxx"
#include "WinClipboard.hxx"
#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>
#include "XNotifyingDataObject.hxx"

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <ole2.h>
#include <objidl.h>

using namespace osl;
using namespace std;
using namespace cppu;

using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::datatransfer::clipboard::RenderingCapabilities;

// definition of static members
CWinClipbImpl* CWinClipbImpl::s_pCWinClipbImpl = nullptr;
osl::Mutex     CWinClipbImpl::s_aMutex;

CWinClipbImpl::CWinClipbImpl( const OUString& aClipboardName, CWinClipboard* theWinClipboard ) :
    m_itsName( aClipboardName ),
    m_pWinClipboard( theWinClipboard ),
    m_pCurrentClipContent( nullptr )
{
    OSL_ASSERT( nullptr != m_pWinClipboard );

    // necessary to reassociate from
    // the static callback function
    s_pCWinClipbImpl = this;
    registerClipboardViewer( );
}

CWinClipbImpl::~CWinClipbImpl( )
{
    {
        MutexGuard aGuard(s_aMutex);
        s_pCWinClipbImpl = nullptr;
    }

    unregisterClipboardViewer( );
}

Reference< XTransferable > CWinClipbImpl::getContents( )
{
    // use the shortcut or create a transferable from
    // system clipboard
    {
        MutexGuard aGuard(m_ClipContentMutex);

        if (nullptr != m_pCurrentClipContent)
        {
            return m_pCurrentClipContent->m_XTransferable;
        }

        // Content cached?
        if (m_foreignContent.is())
            return m_foreignContent;

        // release the mutex, so that the variable may be
        // changed by other threads
    }

    Reference< XTransferable > rClipContent;

    // get the current dataobject from clipboard
    IDataObjectPtr pIDataObject;
    HRESULT hr = m_MtaOleClipboard.getClipboard( &pIDataObject );

    if ( SUCCEEDED( hr ) )
    {
        // create an apartment neutral dataobject and initialize it with a
        // com smart pointer to the IDataObject from clipboard
        IDataObjectPtr pIDo( new CAPNDataObject( pIDataObject ) );

        // remember pIDo destroys itself due to the smart pointer
        rClipContent = CDOTransferable::create( m_pWinClipboard->m_xContext, pIDo );

        MutexGuard aGuard(m_ClipContentMutex);
        m_foreignContent = rClipContent;
    }

    return rClipContent;
}

void CWinClipbImpl::setContents(
    const Reference< XTransferable >& xTransferable,
    const Reference< XClipboardOwner >& xClipboardOwner )
{
    IDataObjectPtr    pIDataObj;

    if ( xTransferable.is( ) )
    {
        {
            MutexGuard aGuard(m_ClipContentMutex);

            m_foreignContent.clear();

            m_pCurrentClipContent
                = new CXNotifyingDataObject(CDTransObjFactory::createDataObjFromTransferable(
                                                m_pWinClipboard->m_xContext, xTransferable),
                                            xTransferable, xClipboardOwner, this);
        }

        pIDataObj = IDataObjectPtr( m_pCurrentClipContent );
    }

    m_MtaOleClipboard.setClipboard(pIDataObj.get());
}

OUString CWinClipbImpl::getName(  )
{
    return m_itsName;
}

sal_Int8 CWinClipbImpl::getRenderingCapabilities(  )
{
    return ( Delayed | Persistent );
}

void CWinClipbImpl::flushClipboard( )
{
    // actually it should be ClearableMutexGuard aGuard( m_ClipContentMutex );
    // but it does not work since FlushClipboard does a callback and frees DataObject
    // which results in a deadlock in onReleaseDataObject.
    // FlushClipboard had to be synchron in order to prevent shutdown until all
    // clipboard-formats are rendered.
    // The request is needed to prevent flushing if we are not clipboard owner (it is
    // not known what happens if we flush but aren't clipboard owner).
    // It may be possible to move the request to the clipboard STA thread by saving the
    // DataObject and call OleIsCurrentClipboard before flushing.

    if ( nullptr != m_pCurrentClipContent )
        m_MtaOleClipboard.flushClipboard( );
}

void CWinClipbImpl::registerClipboardViewer( )
{
    m_MtaOleClipboard.registerClipViewer( CWinClipbImpl::onClipboardContentChanged );
}

void CWinClipbImpl::unregisterClipboardViewer( )
{
    m_MtaOleClipboard.registerClipViewer( nullptr );
}

void CWinClipbImpl::dispose()
{
    OSL_ENSURE( !m_pCurrentClipContent, "Clipboard was not flushed before shutdown!" );
}

void WINAPI CWinClipbImpl::onClipboardContentChanged()
{
    MutexGuard aGuard( s_aMutex );

    // reassociation to instance through static member
    if ( nullptr != s_pCWinClipbImpl )
    {
        s_pCWinClipbImpl->m_foreignContent.clear();
        s_pCWinClipbImpl->m_pWinClipboard->notifyAllClipboardListener( );
    }
}

void CWinClipbImpl::onReleaseDataObject( CXNotifyingDataObject* theCaller )
{
    OSL_ASSERT( nullptr != theCaller );

    if ( theCaller )
        theCaller->lostOwnership( );

    // if the current caller is the one we currently
    // hold, then set it to NULL because an external
    // source must be the clipboardowner now
    MutexGuard aGuard( m_ClipContentMutex );

    if ( m_pCurrentClipContent == theCaller )
        m_pCurrentClipContent = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
