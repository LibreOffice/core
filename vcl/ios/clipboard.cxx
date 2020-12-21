/* -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include "ios/iosinst.hxx"

#include "clipboard.hxx"

#include "DataFlavorMapping.hxx"
#include "iOSTransferable.hxx"
#include <com/sun/star/datatransfer/MimeContentTypeFactory.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>

iOSClipboard::iOSClipboard()
    : WeakComponentImplHelper<XSystemClipboard, XServiceInfo>(m_aMutex)
{
    auto xContext = comphelper::getProcessComponentContext();

    mrXMimeCntFactory = css::datatransfer::MimeContentTypeFactory::create(xContext);

    mpDataFlavorMapper.reset(new DataFlavorMapper());

    mPasteboard = [UIPasteboard generalPasteboard];
    assert(mPasteboard != nil);
}

iOSClipboard::~iOSClipboard() {}

css::uno::Reference<css::datatransfer::XTransferable> SAL_CALL iOSClipboard::getContents()
{
    osl::MutexGuard aGuard(m_aMutex);

    return css::uno::Reference<css::datatransfer::XTransferable>(
        new iOSTransferable(mrXMimeCntFactory, mpDataFlavorMapper, mPasteboard));
}

void SAL_CALL iOSClipboard::setContents(
    const css::uno::Reference<css::datatransfer::XTransferable>& xTransferable,
    const css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner>& /*xClipboardOwner*/)
{
    NSArray* types = xTransferable.is() ? mpDataFlavorMapper->flavorSequenceToTypesArray(
                                              xTransferable->getTransferDataFlavors())
                                        : [NSArray array];

    osl::ClearableMutexGuard aGuard(m_aMutex);

    NSMutableDictionary* dict = [NSMutableDictionary dictionaryWithCapacity:1];
    NSArray* array = @[ dict ];

    for (sal_uInt32 i = 0; i < [types count]; ++i)
    {
        DataProviderPtr_t dp = mpDataFlavorMapper->getDataProvider(types[i], xTransferable);

        if (dp.get() != nullptr)
        {
            NSData* pBoardData = (NSData*)dp->getSystemData();
            dict[types[i]] = pBoardData;
        }
    }
    SAL_INFO("vcl.ios.clipboard", "Setting pasteboard items: " << NSDictionaryKeysToOUString(dict));
    [mPasteboard setItems:array options:@{}];

    // We don't keep a copy of the clipboard contents around in-process, so fire the lost clipboard
    // ownership event right away.
    // fireLostClipboardOwnershipEvent(xClipboardOwner, xTransferable);

    // fireClipboardChangedEvent(xTransferable);
}

OUString SAL_CALL iOSClipboard::getName() { return OUString(); }

sal_Int8 SAL_CALL iOSClipboard::getRenderingCapabilities() { return 0; }

void SAL_CALL iOSClipboard::addClipboardListener(
    const css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>& listener)
{
    osl::MutexGuard aGuard(m_aMutex);

    if (!listener.is())
        throw css::lang::IllegalArgumentException(
            "empty reference", static_cast<css::datatransfer::clipboard::XClipboardEx*>(this), 1);

    mClipboardListeners.push_back(listener);
}

void SAL_CALL iOSClipboard::removeClipboardListener(
    const css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>& listener)
{
    osl::MutexGuard aGuard(m_aMutex);

    if (!listener.is())
        throw css::lang::IllegalArgumentException(
            "empty reference", static_cast<css::datatransfer::clipboard::XClipboardEx*>(this), 1);

    mClipboardListeners.remove(listener);
}

void iOSClipboard::fireClipboardChangedEvent(
    css::uno::Reference<css::datatransfer::XTransferable> xNewContents)
{
    osl::ClearableMutexGuard aGuard(m_aMutex);

    std::list<css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>> listeners(
        mClipboardListeners);
    css::datatransfer::clipboard::ClipboardEvent aEvent;

    if (!listeners.empty())
    {
        aEvent = css::datatransfer::clipboard::ClipboardEvent(static_cast<OWeakObject*>(this),
                                                              xNewContents);
    }

    aGuard.clear();

    while (!listeners.empty())
    {
        if (listeners.front().is())
        {
            try
            {
                listeners.front()->changedContents(aEvent);
            }
            catch (const css::uno::RuntimeException&)
            {
            }
        }
        listeners.pop_front();
    }
}

void iOSClipboard::fireLostClipboardOwnershipEvent(
    css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner> const& oldOwner,
    css::uno::Reference<css::datatransfer::XTransferable> const& oldContent)
{
    assert(oldOwner.is());

    try
    {
        oldOwner->lostOwnership(static_cast<css::datatransfer::clipboard::XClipboardEx*>(this),
                                oldContent);
    }
    catch (const css::uno::RuntimeException&)
    {
    }
}

OUString SAL_CALL iOSClipboard::getImplementationName()
{
    return OUString("com.sun.star.datatransfer.clipboard.iOSClipboard");
}

sal_Bool SAL_CALL iOSClipboard::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SAL_CALL iOSClipboard::getSupportedServiceNames()
{
    return { OUString("com.sun.star.datatransfer.clipboard.SystemClipboard") };
}

css::uno::Reference<css::uno::XInterface>
IosSalInstance::CreateClipboard(const css::uno::Sequence<css::uno::Any>&)
{
    return css::uno::Reference<css::uno::XInterface>(
        static_cast<cppu::OWeakObject*>(new iOSClipboard()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
