/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#pragma once

#include "DataFlavorMapping.hxx"
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/datatransfer/clipboard/XSystemClipboard.hpp>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <list>

#include <premac.h>
#import <UIKit/UIKit.h>
#include <postmac.h>

class iOSClipboard;

@interface PasteboardChangedEventListener : NSObject
{
    iOSClipboard* piOSClipboard;
}
- (PasteboardChangedEventListener*)initWithiOSClipboard:(iOSClipboard*)pcb;
- (void)pasteboardChanged:(NSNotification*)aNotification;
- (void)disposing;
@end

class iOSClipboard
    : public ::cppu::BaseMutex,
      public ::cppu::WeakComponentImplHelper<css::datatransfer::clipboard::XSystemClipboard,
                                             css::lang::XServiceInfo>
{
public:
    iOSClipboard();

    virtual ~iOSClipboard() override;
    iOSClipboard(const iOSClipboard&) = delete;
    iOSClipboard& operator=(const iOSClipboard&) = delete;

    // XClipboard

    css::uno::Reference<css::datatransfer::XTransferable> SAL_CALL getContents() override;

    void SAL_CALL setContents(
        const css::uno::Reference<css::datatransfer::XTransferable>& xTransferable,
        const css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner>& xClipboardOwner)
        override;

    OUString SAL_CALL getName() override;

    // XClipboardEx

    sal_Int8 SAL_CALL getRenderingCapabilities() override;

    // XClipboardNotifier

    void SAL_CALL addClipboardListener(
        const css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>& listener)
        override;

    void SAL_CALL removeClipboardListener(
        const css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>& listener)
        override;

    // XServiceInfo

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    void contentsChanged();

private:
    /* Notify the current clipboard owner that he is no longer the clipboard owner. */
    void fireLostClipboardOwnershipEvent(
        css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner> const& oldOwner,
        css::uno::Reference<css::datatransfer::XTransferable> const& oldContent);

    /* Notify all registered XClipboardListener that the clipboard content has changed. */
    void
    fireClipboardChangedEvent(css::uno::Reference<css::datatransfer::XTransferable> xNewContents);

private:
    css::uno::Reference<css::datatransfer::XMimeContentTypeFactory> mrXMimeCntFactory;
    std::list<css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>>
        mClipboardListeners;
    css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner> mXClipboardOwner;
    std::shared_ptr<DataFlavorMapper> mpDataFlavorMapper;
    NSInteger mnPasteboardChangeCount;
    PasteboardChangedEventListener* mpPasteboardChangedEventListener;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
