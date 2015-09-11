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

#include <X11/Xatom.h>
#include <X11_clipboard.hxx>
#include <X11_transferable.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <uno/dispatcher.h>
#include <uno/mapping.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <rtl/tencinfo.h>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;
using namespace cppu;
using namespace osl;
using namespace x11;

X11Clipboard::X11Clipboard( SelectionManager& rManager, Atom aSelection ) :
        ::cppu::WeakComponentImplHelper<
    ::com::sun::star::datatransfer::clipboard::XSystemClipboard,
    ::com::sun::star::lang::XServiceInfo
    >( rManager.getMutex() ),

        m_rSelectionManager( rManager ),
        m_xSelectionManager( & rManager ),
        m_aSelection( aSelection )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "creating instance of X11Clipboard (this=%p)\n", this );
#endif
}

css::uno::Reference<css::datatransfer::clipboard::XClipboard>
X11Clipboard::create( SelectionManager& rManager, Atom aSelection )
{
    rtl::Reference<X11Clipboard> cb(new X11Clipboard(rManager, aSelection));
    if( aSelection != None )
    {
        rManager.registerHandler( aSelection, *cb.get() );
    }
    else
    {
        rManager.registerHandler( XA_PRIMARY, *cb.get() );
        rManager.registerHandler( rManager.getAtom( OUString("CLIPBOARD") ), *cb.get() );
    }
    return cb.get();
}

X11Clipboard::~X11Clipboard()
{
    MutexGuard aGuard( *Mutex::getGlobalMutex() );

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "shutting down instance of X11Clipboard (this=%p, Selection=\"%s\")\n", this, OUStringToOString( m_rSelectionManager.getString( m_aSelection ), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
    if( m_aSelection != None )
        m_rSelectionManager.deregisterHandler( m_aSelection );
    else
    {
        m_rSelectionManager.deregisterHandler( XA_PRIMARY );
        m_rSelectionManager.deregisterHandler( m_rSelectionManager.getAtom( OUString("CLIPBOARD") ) );
    }
}

void X11Clipboard::fireChangedContentsEvent()
{
    ClearableMutexGuard aGuard( m_rSelectionManager.getMutex() );
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "X11Clipboard::fireChangedContentsEvent for %s (%" SAL_PRI_SIZET "u listeners)\n",
             OUStringToOString( m_rSelectionManager.getString( m_aSelection ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(), m_aListeners.size() );
#endif
    ::std::list< Reference< XClipboardListener > > listeners( m_aListeners );
    aGuard.clear();

    ClipboardEvent aEvent( static_cast<OWeakObject*>(this), m_aContents);
    while( listeners.begin() != listeners.end() )
    {
        if( listeners.front().is() )
            listeners.front()->changedContents(aEvent);
        listeners.pop_front();
    }
}

void X11Clipboard::clearContents()
{
    ClearableMutexGuard aGuard(m_rSelectionManager.getMutex());
    // protect against deletion during outside call
    Reference< XClipboard > xThis( static_cast<XClipboard*>(this));
    // copy member references on stack so they can be called
    // without having the mutex
    Reference< XClipboardOwner > xOwner( m_aOwner );
    Reference< XTransferable > xTrans( m_aContents );
    // clear members
    m_aOwner.clear();
    m_aContents.clear();

    // release the mutex
    aGuard.clear();

    // inform previous owner of lost ownership
    if ( xOwner.is() )
        xOwner->lostOwnership(xThis, m_aContents);
}

Reference< XTransferable > SAL_CALL X11Clipboard::getContents()
    throw(RuntimeException, std::exception)
{
    MutexGuard aGuard(m_rSelectionManager.getMutex());

    if( ! m_aContents.is() )
        m_aContents = new X11Transferable( SelectionManager::get(), m_aSelection );
    return m_aContents;
}

void SAL_CALL X11Clipboard::setContents(
    const Reference< XTransferable >& xTrans,
    const Reference< XClipboardOwner >& xClipboardOwner )
    throw(RuntimeException, std::exception)
{
    // remember old values for callbacks before setting the new ones.
    ClearableMutexGuard aGuard(m_rSelectionManager.getMutex());

    Reference< XClipboardOwner > oldOwner( m_aOwner );
    m_aOwner = xClipboardOwner;

    Reference< XTransferable > oldContents( m_aContents );
    m_aContents = xTrans;

    aGuard.clear();

    // for now request ownership for both selections
    if( m_aSelection != None )
        m_rSelectionManager.requestOwnership( m_aSelection );
    else
    {
        m_rSelectionManager.requestOwnership( XA_PRIMARY );
        m_rSelectionManager.requestOwnership( m_rSelectionManager.getAtom( OUString("CLIPBOARD") ) );
    }

    // notify old owner on loss of ownership
    if( oldOwner.is() )
        oldOwner->lostOwnership(static_cast < XClipboard * > (this), oldContents);

    // notify all listeners on content changes
    fireChangedContentsEvent();
}

OUString SAL_CALL X11Clipboard::getName()
    throw(RuntimeException, std::exception)
{
    return m_rSelectionManager.getString( m_aSelection );
}

sal_Int8 SAL_CALL X11Clipboard::getRenderingCapabilities()
    throw(RuntimeException, std::exception)
{
    return RenderingCapabilities::Delayed;
}

void SAL_CALL X11Clipboard::addClipboardListener( const Reference< XClipboardListener >& listener )
    throw(RuntimeException, std::exception)
{
    MutexGuard aGuard( m_rSelectionManager.getMutex() );
    m_aListeners.push_back( listener );
}

void SAL_CALL X11Clipboard::removeClipboardListener( const Reference< XClipboardListener >& listener )
    throw(RuntimeException, std::exception)
{
    MutexGuard aGuard( m_rSelectionManager.getMutex() );
    m_aListeners.remove( listener );
}

Reference< XTransferable > X11Clipboard::getTransferable()
{
    return getContents();
}

void X11Clipboard::clearTransferable()
{
    clearContents();
}

void X11Clipboard::fireContentsChanged()
{
    fireChangedContentsEvent();
}

Reference< XInterface > X11Clipboard::getReference() throw()
{
    return Reference< XInterface >( static_cast< OWeakObject* >(this) );
}

OUString SAL_CALL X11Clipboard::getImplementationName(  )
    throw(RuntimeException, std::exception)
{
    return OUString(X11_CLIPBOARD_IMPLEMENTATION_NAME);
}

sal_Bool SAL_CALL X11Clipboard::supportsService( const OUString& ServiceName )
    throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL X11Clipboard::getSupportedServiceNames(    )
    throw(RuntimeException, std::exception)
{
    return X11Clipboard_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
