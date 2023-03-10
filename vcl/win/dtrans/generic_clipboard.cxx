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

#include "generic_clipboard.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;

using ::dtrans::GenericClipboard;

GenericClipboard::GenericClipboard() :
    m_bInitialized(false)
{
}

GenericClipboard::~GenericClipboard()
{
}

void SAL_CALL GenericClipboard::initialize( const Sequence< Any >& aArguments )
{
    if (!m_bInitialized)
    {
        for (Any const & arg : aArguments)
            if (arg.getValueType() == cppu::UnoType<OUString>::get())
            {
                arg >>= m_aName;
                break;
            }
    }
}

OUString SAL_CALL GenericClipboard::getImplementationName(  )
{
    return "com.sun.star.comp.datatransfer.clipboard.GenericClipboard";
}

sal_Bool SAL_CALL GenericClipboard::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL GenericClipboard::getSupportedServiceNames(    )
{
    return { "com.sun.star.datatransfer.clipboard.GenericClipboard" };
}

Reference< XTransferable > SAL_CALL GenericClipboard::getContents()
{
    std::unique_lock aGuard(m_aMutex);
    return m_aContents;
}

void SAL_CALL GenericClipboard::setContents(const Reference< XTransferable >& xTrans,
                                      const Reference< XClipboardOwner >& xClipboardOwner )
{
    // remember old values for callbacks before setting the new ones.
    std::unique_lock aGuard(m_aMutex);

    Reference< XClipboardOwner > oldOwner(m_aOwner);
    m_aOwner = xClipboardOwner;

    Reference< XTransferable > oldContents(m_aContents);
    m_aContents = xTrans;

    aGuard.unlock();

    // notify old owner on loss of ownership
    if( oldOwner.is() )
        oldOwner->lostOwnership(static_cast < XClipboard * > (this), oldContents);

    // notify all listeners on content changes
    aGuard.lock();
    ClipboardEvent aEvent(static_cast < XClipboard * > (this), m_aContents);
    maClipboardListeners.notifyEach(aGuard, &XClipboardListener::changedContents, aEvent);
}

OUString SAL_CALL GenericClipboard::getName()
{
    return m_aName;
}

sal_Int8 SAL_CALL GenericClipboard::getRenderingCapabilities()
{
    return RenderingCapabilities::Delayed;
}

void SAL_CALL GenericClipboard::addClipboardListener( const Reference< XClipboardListener >& listener )
{
    std::unique_lock aGuard( m_aMutex );
    OSL_ENSURE( !m_bDisposed, "object is disposed" );
    if (!m_bDisposed)
        maClipboardListeners.addInterface( aGuard, listener );
}

void SAL_CALL GenericClipboard::removeClipboardListener( const Reference< XClipboardListener >& listener )
{
    std::unique_lock aGuard( m_aMutex );
    OSL_ENSURE( !m_bDisposed, "object is disposed" );
    if (!m_bDisposed)
        maClipboardListeners.removeInterface( aGuard, listener );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
dtrans_GenericClipboard_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new GenericClipboard());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
