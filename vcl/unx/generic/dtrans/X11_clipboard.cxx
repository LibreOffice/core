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
#include "X11_clipboard.hxx"
#include "X11_transferable.hxx"
#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace x11;

X11Clipboard::X11Clipboard( SelectionManager& rManager, Atom aSelection ) :
        ::cppu::WeakComponentImplHelper<
    css::datatransfer::clipboard::XSystemClipboard,
    css::lang::XServiceInfo
    >( rManager.getMutex() ),

        m_xSelectionManager( &rManager ),
        m_aSelection( aSelection )
{
#if OSL_DEBUG_LEVEL > 1
    SAL_INFO("vcl.unx.dtrans", "creating instance of X11Clipboard (this="
            << this << ").");
#endif
}

css::uno::Reference<css::datatransfer::clipboard::XClipboard>
X11Clipboard::create( SelectionManager& rManager, Atom aSelection )
{
    rtl::Reference<X11Clipboard> cb(new X11Clipboard(rManager, aSelection));
    if( aSelection != None )
    {
        rManager.registerHandler(aSelection, *cb);
    }
    else
    {
        rManager.registerHandler(XA_PRIMARY, *cb);
        rManager.registerHandler(rManager.getAtom(u"CLIPBOARD"_ustr), *cb);
    }
    return cb;
}

X11Clipboard::~X11Clipboard()
{
    MutexGuard aGuard( *Mutex::getGlobalMutex() );

#if OSL_DEBUG_LEVEL > 1
    SAL_INFO("vcl.unx.dtrans", "shutting down instance of X11Clipboard (this="
            << this
            << ", Selection=\""
            << m_xSelectionManager->getString( m_aSelection )
            << "\").");
#endif

    if( m_aSelection != None )
        m_xSelectionManager->deregisterHandler( m_aSelection );
    else
    {
        m_xSelectionManager->deregisterHandler( XA_PRIMARY );
        m_xSelectionManager->deregisterHandler( m_xSelectionManager->getAtom( u"CLIPBOARD"_ustr ) );
    }
}

void X11Clipboard::fireChangedContentsEvent()
{
    ClearableMutexGuard aGuard( m_xSelectionManager->getMutex() );
#if OSL_DEBUG_LEVEL > 1
    SAL_INFO("vcl.unx.dtrans", "X11Clipboard::fireChangedContentsEvent for "
            << m_xSelectionManager->getString( m_aSelection )
            << " (" << m_aListeners.size() << " listeners).");
#endif
    ::std::vector< Reference< XClipboardListener > > listeners( m_aListeners );
    aGuard.clear();

    ClipboardEvent aEvent(getXWeak(), m_aContents);
    for (auto const& listener : listeners)
    {
        if( listener.is() )
            listener->changedContents(aEvent);
    }
}

void X11Clipboard::clearContents()
{
    ClearableMutexGuard aGuard(m_xSelectionManager->getMutex());
    // protect against deletion during outside call
    Reference< XClipboard > xThis( static_cast<XClipboard*>(this));
    // copy member references on stack so they can be called
    // without having the mutex
    Reference< XClipboardOwner > xOwner( m_aOwner );
    Reference< XTransferable > xKeepAlive( m_aContents );
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
{
    MutexGuard aGuard(m_xSelectionManager->getMutex());

    if( ! m_aContents.is() )
        m_aContents = new X11Transferable( SelectionManager::get(), m_aSelection );
    return m_aContents;
}

void SAL_CALL X11Clipboard::setContents(
    const Reference< XTransferable >& xTrans,
    const Reference< XClipboardOwner >& xClipboardOwner )
{
    // remember old values for callbacks before setting the new ones.
    ClearableMutexGuard aGuard(m_xSelectionManager->getMutex());

    Reference< XClipboardOwner > oldOwner( m_aOwner );
    m_aOwner = xClipboardOwner;

    Reference< XTransferable > oldContents( m_aContents );
    m_aContents = xTrans;

    aGuard.clear();

    // for now request ownership for both selections
    if( m_aSelection != None )
        m_xSelectionManager->requestOwnership( m_aSelection );
    else
    {
        m_xSelectionManager->requestOwnership( XA_PRIMARY );
        m_xSelectionManager->requestOwnership( m_xSelectionManager->getAtom( u"CLIPBOARD"_ustr ) );
    }

    // notify old owner on loss of ownership
    if( oldOwner.is() )
        oldOwner->lostOwnership(static_cast < XClipboard * > (this), oldContents);

    // notify all listeners on content changes
    fireChangedContentsEvent();
}

OUString SAL_CALL X11Clipboard::getName()
{
    return m_xSelectionManager->getString( m_aSelection );
}

sal_Int8 SAL_CALL X11Clipboard::getRenderingCapabilities()
{
    return RenderingCapabilities::Delayed;
}

void SAL_CALL X11Clipboard::addClipboardListener( const Reference< XClipboardListener >& listener )
{
    MutexGuard aGuard( m_xSelectionManager->getMutex() );
    m_aListeners.push_back( listener );
}

void SAL_CALL X11Clipboard::removeClipboardListener( const Reference< XClipboardListener >& listener )
{
    MutexGuard aGuard( m_xSelectionManager->getMutex() );
    std::erase(m_aListeners, listener);
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

Reference< XInterface > X11Clipboard::getReference() noexcept
{
    return getXWeak();
}

OUString SAL_CALL X11Clipboard::getImplementationName(  )
{
    return X11_CLIPBOARD_IMPLEMENTATION_NAME;
}

sal_Bool SAL_CALL X11Clipboard::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL X11Clipboard::getSupportedServiceNames(    )
{
    return X11Clipboard_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
