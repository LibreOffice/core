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

#include <clipboardmanager.hxx>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>

using namespace com::sun::star::container;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace std;

using ::dtrans::ClipboardManager;

ClipboardManager::ClipboardManager():
    WeakComponentImplHelper< XClipboardManager, XEventListener, XServiceInfo > (m_aMutex),
    m_aDefaultName(OUString("default"))
{
}

ClipboardManager::~ClipboardManager()
{
}

OUString SAL_CALL ClipboardManager::getImplementationName(  )
{
    return OUString(CLIPBOARDMANAGER_IMPLEMENTATION_NAME);
}

sal_Bool SAL_CALL ClipboardManager::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL ClipboardManager::getSupportedServiceNames(  )
{
    return ClipboardManager_getSupportedServiceNames();
}

Reference< XClipboard > SAL_CALL ClipboardManager::getClipboard( const OUString& aName )
{
    MutexGuard aGuard(m_aMutex);

    // object is disposed already
    if (rBHelper.bDisposed)
        throw DisposedException("object is disposed.",
                                static_cast < XClipboardManager * > (this));

    ClipboardMap::iterator iter =
        m_aClipboardMap.find(aName.getLength() ? aName : m_aDefaultName);

    if (iter != m_aClipboardMap.end())
        return iter->second;

    throw NoSuchElementException(aName, static_cast < XClipboardManager * > (this));
}

void SAL_CALL ClipboardManager::addClipboard( const Reference< XClipboard >& xClipboard )
{
    OSL_ASSERT(xClipboard.is());

    // check parameter
    if (!xClipboard.is())
        throw IllegalArgumentException("empty reference",
                                       static_cast < XClipboardManager * > (this), 1);

    // the name "default" is reserved for internal use
    OUString aName = xClipboard->getName();
    if ( m_aDefaultName == aName )
        throw IllegalArgumentException("name reserved",
                                       static_cast < XClipboardManager * > (this), 1);

    // try to add new clipboard to the list
    ClearableMutexGuard aGuard(m_aMutex);
    if (!rBHelper.bDisposed && !rBHelper.bInDispose)
    {
        pair< const OUString, Reference< XClipboard > > value (
            aName.getLength() ? aName : m_aDefaultName,
            xClipboard );

        pair< ClipboardMap::iterator, bool > p = m_aClipboardMap.insert(value);
        aGuard.clear();

        // insert failed, element must exist already
        if (!p.second)
            throw ElementExistException(aName, static_cast < XClipboardManager * > (this));

        // request disposing notifications
        Reference< XComponent > xComponent(xClipboard, UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener(static_cast < XEventListener * > (this));
    }
}

void SAL_CALL ClipboardManager::removeClipboard( const OUString& aName )
{
    MutexGuard aGuard(m_aMutex);
    if (!rBHelper.bDisposed)
        m_aClipboardMap.erase(aName.getLength() ? aName : m_aDefaultName );
}

Sequence< OUString > SAL_CALL ClipboardManager::listClipboardNames()
{
    MutexGuard aGuard(m_aMutex);

    if (rBHelper.bDisposed)
        throw DisposedException("object is disposed.",
                                static_cast < XClipboardManager * > (this));

    if (rBHelper.bInDispose)
        return Sequence< OUString > ();

    return comphelper::mapKeysToSequence(m_aClipboardMap);
}

void SAL_CALL ClipboardManager::dispose()
{
    ClearableMutexGuard aGuard( rBHelper.rMutex );
    if (!rBHelper.bDisposed && !rBHelper.bInDispose)
    {
        rBHelper.bInDispose = true;
        aGuard.clear();

        // give everyone a chance to save his clipboard instance
        EventObject aEvt(static_cast < XClipboardManager * > (this));
        rBHelper.aLC.disposeAndClear( aEvt );

        // removeClipboard is still allowed here,  so make a copy of the
        // list (to ensure integrity) and clear the original.
        ClearableMutexGuard aGuard2( rBHelper.rMutex );
        ClipboardMap aCopy(m_aClipboardMap);
        m_aClipboardMap.clear();
        aGuard2.clear();

        // dispose all clipboards still in list
        ClipboardMap::iterator iter = aCopy.begin();
        ClipboardMap::iterator imax = aCopy.end();

        for (; iter != imax; ++iter)
        {
            Reference< XComponent > xComponent(iter->second, UNO_QUERY);
            if (xComponent.is())
            {
                try
                {
                    xComponent->removeEventListener(static_cast < XEventListener * > (this));
                    xComponent->dispose();
                }
                catch (const Exception&)
                {
                    // exceptions can be safely ignored here.
                }
            }
        }

        rBHelper.bDisposed = true;
        rBHelper.bInDispose = false;
    }
}

void SAL_CALL  ClipboardManager::disposing( const EventObject& event )
{
    Reference < XClipboard > xClipboard(event.Source, UNO_QUERY);

    if (xClipboard.is())
        removeClipboard(xClipboard->getName());
}

Reference< XInterface > SAL_CALL ClipboardManager_createInstance(
    const Reference< XMultiServiceFactory > & /*xMultiServiceFactory*/)
{
    return Reference < XInterface >(static_cast<OWeakObject *>(new ClipboardManager()));
}

Sequence< OUString > SAL_CALL ClipboardManager_getSupportedServiceNames()
{
    Sequence < OUString > SupportedServicesNames { "com.sun.star.datatransfer.clipboard.ClipboardManager" };
    return SupportedServicesNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
