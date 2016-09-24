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

#include <generic_clipboard.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>
#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;

using ::dtrans::GenericClipboard;

GenericClipboard::GenericClipboard() :
    WeakComponentImplHelper< XClipboardEx, XClipboardNotifier, XServiceInfo, XInitialization > (m_aMutex),
    m_bInitialized(sal_False)
{
}

GenericClipboard::~GenericClipboard()
{
}

void SAL_CALL GenericClipboard::initialize( const Sequence< Any >& aArguments )
    throw(Exception, RuntimeException)
{
    if (!m_bInitialized)
    {
        for (sal_Int32 n = 0, nmax = aArguments.getLength(); n < nmax; n++)
            if (aArguments[n].getValueType() == cppu::UnoType<OUString>::get())
            {
                aArguments[0] >>= m_aName;
                break;
            }
    }
}

OUString SAL_CALL GenericClipboard::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString(GENERIC_CLIPBOARD_IMPLEMENTATION_NAME);
}

sal_Bool SAL_CALL GenericClipboard::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL GenericClipboard::getSupportedServiceNames(    )
    throw(RuntimeException)
{
    return GenericClipboard_getSupportedServiceNames();
}

Reference< XTransferable > SAL_CALL GenericClipboard::getContents()
    throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return m_aContents;
}

void SAL_CALL GenericClipboard::setContents(const Reference< XTransferable >& xTrans,
                                      const Reference< XClipboardOwner >& xClipboardOwner )
    throw(RuntimeException)
{
    // remember old values for callbacks before setting the new ones.
    ClearableMutexGuard aGuard(m_aMutex);

    Reference< XClipboardOwner > oldOwner(m_aOwner);
    m_aOwner = xClipboardOwner;

    Reference< XTransferable > oldContents(m_aContents);
    m_aContents = xTrans;

    aGuard.clear();

    // notify old owner on loss of ownership
    if( oldOwner.is() )
        oldOwner->lostOwnership(static_cast < XClipboard * > (this), oldContents);

    // notify all listeners on content changes
    OInterfaceContainerHelper *pContainer =
        rBHelper.aLC.getContainer(cppu::UnoType<XClipboardListener>::get());
    if (pContainer)
    {
        ClipboardEvent aEvent(static_cast < XClipboard * > (this), m_aContents);
        OInterfaceIteratorHelper aIterator(*pContainer);

        while (aIterator.hasMoreElements())
        {
            Reference < XClipboardListener > xListener(aIterator.next(), UNO_QUERY);
            if (xListener.is())
                xListener->changedContents(aEvent);
        }
    }
}

OUString SAL_CALL GenericClipboard::getName()
    throw(RuntimeException)
{
    return m_aName;
}

sal_Int8 SAL_CALL GenericClipboard::getRenderingCapabilities()
    throw(RuntimeException)
{
    return RenderingCapabilities::Delayed;
}

void SAL_CALL GenericClipboard::addClipboardListener( const Reference< XClipboardListener >& listener )
    throw(RuntimeException)
{
    MutexGuard aGuard( rBHelper.rMutex );
    OSL_ENSURE( !rBHelper.bInDispose, "do not add listeners in the dispose call" );
    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    if (!rBHelper.bInDispose && !rBHelper.bDisposed)
        rBHelper.aLC.addInterface( cppu::UnoType<XClipboardListener>::get(), listener );
}

void SAL_CALL GenericClipboard::removeClipboardListener( const Reference< XClipboardListener >& listener )
    throw(RuntimeException)
{
    MutexGuard aGuard( rBHelper.rMutex );
    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    if (!rBHelper.bInDispose && !rBHelper.bDisposed)
        rBHelper.aLC.removeInterface( cppu::UnoType<XClipboardListener>::get(), listener ); \
}

Sequence< OUString > SAL_CALL GenericClipboard_getSupportedServiceNames()
{
    Sequence< OUString > aRet { "com.sun.star.datatransfer.clipboard.GenericClipboard" };
    return aRet;
}

Reference< XInterface > SAL_CALL GenericClipboard_createInstance(
    const Reference< XMultiServiceFactory > & /*xMultiServiceFactory*/)
{
    return Reference < XInterface >( ( OWeakObject * ) new GenericClipboard());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
