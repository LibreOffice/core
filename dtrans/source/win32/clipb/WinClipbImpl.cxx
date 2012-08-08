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
#include "../../inc/DtObjFactory.hxx"
#include "../dtobj/APNDataObject.hxx"
#include "WinClipboard.hxx"
#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>
#include "../dtobj/XNotifyingDataObject.hxx"

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#include <ole2.h>
#include <objidl.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace osl;
using namespace std;
using namespace cppu;

using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::datatransfer::clipboard::RenderingCapabilities;

using ::rtl::OUString;

// definition of static members
CWinClipbImpl* CWinClipbImpl::s_pCWinClipbImpl = NULL;
osl::Mutex     CWinClipbImpl::s_aMutex;

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CWinClipbImpl::CWinClipbImpl( const OUString& aClipboardName, CWinClipboard* theWinClipboard ) :
    m_itsName( aClipboardName ),
    m_pWinClipboard( theWinClipboard ),
    m_pCurrentClipContent( NULL )
{
    OSL_ASSERT( NULL != m_pWinClipboard );

    // necessary to reassociate from
    // the static callback function
    s_pCWinClipbImpl = this;
    registerClipboardViewer( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CWinClipbImpl::~CWinClipbImpl( )
{
    ClearableMutexGuard aGuard( s_aMutex );
    s_pCWinClipbImpl = NULL;
    aGuard.clear( );

    unregisterClipboardViewer( );
}

//------------------------------------------------------------------------
// getContent
//------------------------------------------------------------------------

Reference< XTransferable > SAL_CALL CWinClipbImpl::getContents( ) throw( RuntimeException )
{
    // use the shotcut or create a transferable from
    // system clipboard
    ClearableMutexGuard aGuard( m_ClipContentMutex );

    if ( NULL != m_pCurrentClipContent )
    {
        return m_pCurrentClipContent->m_XTransferable;
    }

    // release the mutex, so that the variable may be
    // changed by other threads
    aGuard.clear( );

    Reference< XTransferable > rClipContent;

    // get the current dataobject from clipboard
    IDataObjectPtr pIDataObject;
    HRESULT hr = m_MtaOleClipboard.getClipboard( &pIDataObject );

    if ( SUCCEEDED( hr ) )
    {
        // create an apartment neutral dataobject and initialize it with a
        // com smart pointer to the IDataObject from clipboard
        IDataObjectPtr pIDo( new CAPNDataObject( pIDataObject ) );

        CDTransObjFactory objFactory;

        // remeber pIDo destroys itself due to the smart pointer
        rClipContent = objFactory.createTransferableFromDataObj( m_pWinClipboard->m_SrvMgr, pIDo );
    }

    return rClipContent;
}

//------------------------------------------------------------------------
// setContent
//------------------------------------------------------------------------

void SAL_CALL CWinClipbImpl::setContents(
    const Reference< XTransferable >& xTransferable,
    const Reference< XClipboardOwner >& xClipboardOwner )
    throw( RuntimeException )
{
    CDTransObjFactory objFactory;
    IDataObjectPtr    pIDataObj;

    if ( xTransferable.is( ) )
    {
        ClearableMutexGuard aGuard( m_ClipContentMutex );

        m_pCurrentClipContent = new CXNotifyingDataObject(
            objFactory.createDataObjFromTransferable( m_pWinClipboard->m_SrvMgr , xTransferable ),
            xTransferable,
            xClipboardOwner,
            this );

        aGuard.clear( );

        pIDataObj = IDataObjectPtr( m_pCurrentClipContent );
    }

    m_MtaOleClipboard.setClipboard(pIDataObj.get());
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CWinClipbImpl::getName(  ) throw( RuntimeException )
{
    return m_itsName;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Int8 SAL_CALL CWinClipbImpl::getRenderingCapabilities(  ) throw( RuntimeException )
{
    return ( Delayed | Persistant );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CWinClipbImpl::flushClipboard( ) throw( RuntimeException )
{
    // sollte eigentlich hier stehen: ClearableMutexGuard aGuard( m_ClipContentMutex );
    // geht aber nicht, da FlushClipboard zurückruft und das DataObject
    // freigibt und damit würde es einen Deadlock in onReleaseDataObject geben
    // FlushClipboard muß synchron sein, damit das runterfahren ggf. erst weitergeht,
    // wenn alle Clipboard-Formate gerendert wurden
    // die Abfrage ist nötig, damit nur geflusht wird, wenn wir wirklich Clipboardowner
    // sind (ich weiss nicht genau was passiert, wenn man flusht und nicht Clipboard
    // owner ist).
    // eventuell kann man aber die Abfrage in den Clipboard STA Thread verlagern, indem
    // man sich dort das DataObject merkt und vor dem flushen OleIsCurrentClipboard ruft

    if ( NULL != m_pCurrentClipContent )
        m_MtaOleClipboard.flushClipboard( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CWinClipbImpl::registerClipboardViewer( )
{
    m_MtaOleClipboard.registerClipViewer( CWinClipbImpl::onClipboardContentChanged );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CWinClipbImpl::unregisterClipboardViewer( )
{
    m_MtaOleClipboard.registerClipViewer( NULL );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CWinClipbImpl::dispose() throw( RuntimeException )
{
    OSL_ENSURE( !m_pCurrentClipContent, "Clipboard was not flushed before shutdown!" );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void WINAPI CWinClipbImpl::onClipboardContentChanged( void )
{
    MutexGuard aGuard( s_aMutex );

    // reassocition to instance through static member
    if ( NULL != s_pCWinClipbImpl )
        s_pCWinClipbImpl->m_pWinClipboard->notifyAllClipboardListener( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CWinClipbImpl::onReleaseDataObject( CXNotifyingDataObject* theCaller )
{
    OSL_ASSERT( NULL != theCaller );

    if ( theCaller )
        theCaller->lostOwnership( );

    // if the current caller is the one we currently
    // hold, then set it to NULL because an external
    // source must be the clipboardowner now
    MutexGuard aGuard( m_ClipContentMutex );

    if ( m_pCurrentClipContent == theCaller )
        m_pCurrentClipContent = NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
