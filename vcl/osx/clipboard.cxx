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

#include "clipboard.hxx"

#include "DataFlavorMapping.hxx"
#include "OSXTransferable.hxx"
#include <com/sun/star/datatransfer/MimeContentTypeFactory.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace css;

@implementation EventListener;

-(EventListener*)initWithAquaClipboard: (AquaClipboard*) pcb
{
    self = [super init];

    if (self)
        pAquaClipboard = pcb;

    return self;
}

-(void)pasteboard:(NSPasteboard*)sender provideDataForType:(const NSString*)type
{
    if( pAquaClipboard )
        pAquaClipboard->provideDataForType(sender, type);
}

-(void)applicationDidBecomeActive:(NSNotification*)aNotification
{
    if( pAquaClipboard )
        pAquaClipboard->applicationDidBecomeActive(aNotification);
}

-(void)disposing
{
    pAquaClipboard = nullptr;
}

@end

static OUString clipboard_getImplementationName()
{
  return "com.sun.star.datatransfer.clipboard.AquaClipboard";
}

static uno::Sequence<OUString> clipboard_getSupportedServiceNames()
{
  return { OUString("com.sun.star.datatransfer.clipboard.SystemClipboard") };
}

AquaClipboard::AquaClipboard(NSPasteboard* pasteboard, bool bUseSystemPasteboard)
    : WeakComponentImplHelper<XSystemClipboard, XFlushableClipboard, XServiceInfo>(m_aMutex)
    , mIsSystemPasteboard(bUseSystemPasteboard)
{
    uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();

    mrXMimeCntFactory = datatransfer::MimeContentTypeFactory::create(xContext);

    mpDataFlavorMapper = DataFlavorMapperPtr_t(new DataFlavorMapper());

    if (pasteboard != nullptr)
    {
      mPasteboard = pasteboard;
      mIsSystemPasteboard = false;
    }
    else
    {
      SAL_WNODEPRECATED_DECLARATIONS_PUSH //TODO: 10.13 NSDragPboard
      mPasteboard = bUseSystemPasteboard ? [NSPasteboard generalPasteboard] :
        [NSPasteboard pasteboardWithName: NSDragPboard];
      SAL_WNODEPRECATED_DECLARATIONS_POP

      if (mPasteboard == nil)
        {
            throw uno::RuntimeException("AquaClipboard: Cannot create Cocoa pasteboard",
                static_cast<XClipboardEx*>(this));
        }
    }

    [mPasteboard retain];

    mEventListener = [[EventListener alloc] initWithAquaClipboard: this];

    if (mEventListener == nil)
    {
        [mPasteboard release];

        throw uno::RuntimeException(
            "AquaClipboard: Cannot create pasteboard change listener",
            static_cast<XClipboardEx*>(this));
    }

    if (mIsSystemPasteboard)
    {
        NSNotificationCenter* notificationCenter = [NSNotificationCenter defaultCenter];

        [notificationCenter addObserver: mEventListener
         selector: @selector(applicationDidBecomeActive:)
         name: @"NSApplicationDidBecomeActiveNotification"
         object: [NSApplication sharedApplication]];
    }

    mPasteboardChangeCount = [mPasteboard changeCount];
}

AquaClipboard::~AquaClipboard()
{
    if (mIsSystemPasteboard)
    {
        [[NSNotificationCenter defaultCenter] removeObserver: mEventListener];
    }

    [mEventListener disposing];
    [mEventListener release];
    [mPasteboard release];
}

uno::Reference<datatransfer::XTransferable> SAL_CALL AquaClipboard::getContents()
{
    osl::MutexGuard aGuard(m_aMutex);

    // Shortcut: If we are clipboard owner already we don't need
    // to drag the data through the system clipboard
    if (mXClipboardContent.is())
    {
        return mXClipboardContent;
    }

    return uno::Reference<datatransfer::XTransferable>(
                new OSXTransferable(mrXMimeCntFactory,
                                    mpDataFlavorMapper,
                                    mPasteboard));
}

void SAL_CALL AquaClipboard::setContents(
    uno::Reference<datatransfer::XTransferable> const & xTransferable,
    uno::Reference<datatransfer::clipboard::XClipboardOwner> const & xClipboardOwner)
{
    NSArray* types = xTransferable.is() ?
        mpDataFlavorMapper->flavorSequenceToTypesArray(xTransferable->getTransferDataFlavors()) :
        [NSArray array];

    osl::ClearableMutexGuard aGuard(m_aMutex);

    uno::Reference<datatransfer::clipboard::XClipboardOwner> oldOwner(mXClipboardOwner);
    mXClipboardOwner = xClipboardOwner;

    uno::Reference<datatransfer::XTransferable> oldContent(mXClipboardContent);
    mXClipboardContent = xTransferable;

    mPasteboardChangeCount = [mPasteboard declareTypes: types owner: mEventListener];

    aGuard.clear();

    // if we are already the owner of the clipboard
    // then fire lost ownership event
    if (oldOwner.is())
    {
        fireLostClipboardOwnershipEvent(oldOwner, oldContent);
    }

    fireClipboardChangedEvent();
}

OUString SAL_CALL AquaClipboard::getName()
{
    return OUString();
}

sal_Int8 SAL_CALL AquaClipboard::getRenderingCapabilities()
{
    return 0;
}

void SAL_CALL AquaClipboard::addClipboardListener(uno::Reference<datatransfer::clipboard::XClipboardListener> const & listener)
{
    osl::MutexGuard aGuard(m_aMutex);

    if (!listener.is())
        throw lang::IllegalArgumentException("empty reference",
                                   static_cast<XClipboardEx*>(this), 1);

    mClipboardListeners.push_back(listener);
}

void SAL_CALL AquaClipboard::removeClipboardListener(uno::Reference<datatransfer::clipboard::XClipboardListener> const & listener)
{
    osl::MutexGuard aGuard(m_aMutex);

    if (!listener.is())
        throw lang::IllegalArgumentException("empty reference",
                                   static_cast<XClipboardEx*>(this), 1);

    mClipboardListeners.remove(listener);
}

void AquaClipboard::applicationDidBecomeActive(NSNotification*)
{
    osl::ClearableMutexGuard aGuard(m_aMutex);

    int currentPboardChgCount = [mPasteboard changeCount];

    if (currentPboardChgCount != mPasteboardChangeCount)
    {
        mPasteboardChangeCount = currentPboardChgCount;

        // Clear clipboard content and owner and send lostOwnership
        // notification to the old clipboard owner as well as
        // ClipboardChanged notification to any clipboard listener
        uno::Reference<datatransfer::clipboard::XClipboardOwner> oldOwner(mXClipboardOwner);
        mXClipboardOwner.clear();

        uno::Reference<datatransfer::XTransferable> oldContent(mXClipboardContent);
        mXClipboardContent.clear();

        aGuard.clear();

        if (oldOwner.is())
        {
            fireLostClipboardOwnershipEvent(oldOwner, oldContent);
        }

        fireClipboardChangedEvent();
    }
}

void AquaClipboard::fireClipboardChangedEvent()
{
    osl::ClearableMutexGuard aGuard(m_aMutex);

    datatransfer::clipboard::ClipboardEvent aEvent;

    if (!mClipboardListeners.empty())
    {
        aEvent = datatransfer::clipboard::ClipboardEvent(static_cast<OWeakObject*>(this), getContents());
    }

    aGuard.clear();

    for (auto const& rListener : mClipboardListeners)
    {
        if (rListener.is())
        {
            try
            {
                rListener->changedContents(aEvent);
            }
            catch (uno::RuntimeException& )
            {}
        }
    }
}

void AquaClipboard::fireLostClipboardOwnershipEvent(
    uno::Reference<datatransfer::clipboard::XClipboardOwner> const & rOldOwner,
    uno::Reference<datatransfer::XTransferable> const & rOldContent)
{
    assert(rOldOwner.is());

    try
    {
        rOldOwner->lostOwnership(static_cast<XClipboardEx*>(this), rOldContent);
    }
    catch(uno::RuntimeException&)
    {}
}

void AquaClipboard::provideDataForType(NSPasteboard* sender, const NSString* type)
{
    if( mXClipboardContent.is() )
    {
        DataProviderPtr_t dp = mpDataFlavorMapper->getDataProvider(type, mXClipboardContent);
        NSData* pBoardData = nullptr;

        if (dp.get() != nullptr)
        {
            pBoardData = dp->getSystemData();
            [sender setData: pBoardData forType:const_cast<NSString*>(type)];
        }
    }
}

void SAL_CALL AquaClipboard::flushClipboard()
{
    if (mXClipboardContent.is())
    {
        uno::Sequence<datatransfer::DataFlavor> flavorList = mXClipboardContent->getTransferDataFlavors();
        sal_uInt32 nFlavors = flavorList.getLength();
        bool bInternal(false);

        for (sal_uInt32 i = 0; i < nFlavors; i++)
        {
            const NSString* sysType = mpDataFlavorMapper->openOfficeToSystemFlavor(flavorList[i], bInternal);

            if (sysType != nullptr)
            {
                provideDataForType(mPasteboard, sysType);
            }
        }
        mXClipboardContent.clear();
    }
}

NSPasteboard* AquaClipboard::getPasteboard() const
{
    return mPasteboard;
}

OUString SAL_CALL AquaClipboard::getImplementationName()
{
    return clipboard_getImplementationName();
}

sal_Bool SAL_CALL AquaClipboard::supportsService(OUString const & rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL AquaClipboard::getSupportedServiceNames()
{
    return clipboard_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
