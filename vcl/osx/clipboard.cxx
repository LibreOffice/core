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
#include "comphelper/processfactory.hxx"
#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace std;
using namespace comphelper;

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

OUString clipboard_getImplementationName()
{
  return OUString("com.sun.star.datatransfer.clipboard.AquaClipboard");
}

Sequence<OUString> clipboard_getSupportedServiceNames()
{
  return { OUString("com.sun.star.datatransfer.clipboard.SystemClipboard") };
}

AquaClipboard::AquaClipboard(NSPasteboard* pasteboard, bool bUseSystemPasteboard) :
  WeakComponentImplHelper<XSystemClipboard, XFlushableClipboard, XServiceInfo>(m_aMutex),
  mIsSystemPasteboard(bUseSystemPasteboard)
{
    Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();

    mrXMimeCntFactory = MimeContentTypeFactory::create(xContext);

    mpDataFlavorMapper = DataFlavorMapperPtr_t(new DataFlavorMapper());

    if (pasteboard != nullptr)
    {
      mPasteboard = pasteboard;
      mIsSystemPasteboard = false;
    }
    else
    {
      mPasteboard = bUseSystemPasteboard ? [NSPasteboard generalPasteboard] :
        [NSPasteboard pasteboardWithName: NSDragPboard];

      if (mPasteboard == nil)
        {
            throw RuntimeException("AquaClipboard: Cannot create Cocoa pasteboard",
                static_cast<XClipboardEx*>(this));
        }
    }

    [mPasteboard retain];

    mEventListener = [[EventListener alloc] initWithAquaClipboard: this];

    if (mEventListener == nil)
    {
        [mPasteboard release];

        throw RuntimeException(
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

Reference<XTransferable> SAL_CALL AquaClipboard::getContents() throw(RuntimeException, std::exception)
{
    MutexGuard aGuard(m_aMutex);

    // Shortcut: If we are clipboard owner already we don't need
    // to drag the data through the system clipboard
    if (mXClipboardContent.is())
    {
        return mXClipboardContent;
    }

    return Reference<XTransferable>(new OSXTransferable(mrXMimeCntFactory,
                                                        mpDataFlavorMapper,
                                                        mPasteboard));
}

void SAL_CALL AquaClipboard::setContents(const Reference<XTransferable>& xTransferable,
    const Reference<XClipboardOwner>& xClipboardOwner)
        throw( RuntimeException, std::exception )
{
    NSArray* types = xTransferable.is() ?
        mpDataFlavorMapper->flavorSequenceToTypesArray(xTransferable->getTransferDataFlavors()) :
        [NSArray array];

    ClearableMutexGuard aGuard(m_aMutex);

    Reference<XClipboardOwner> oldOwner(mXClipboardOwner);
    mXClipboardOwner = xClipboardOwner;

    Reference<XTransferable> oldContent(mXClipboardContent);
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

OUString SAL_CALL AquaClipboard::getName() throw( RuntimeException, std::exception )
{
    return OUString();
}

sal_Int8 SAL_CALL AquaClipboard::getRenderingCapabilities() throw( RuntimeException, std::exception )
{
    return 0;
}

void SAL_CALL AquaClipboard::addClipboardListener(const Reference< XClipboardListener >& listener)
  throw( RuntimeException, std::exception )
{
  MutexGuard aGuard(m_aMutex);

  if (!listener.is())
     throw IllegalArgumentException("empty reference",
                                   static_cast<XClipboardEx*>(this), 1);

  mClipboardListeners.push_back(listener);
}

void SAL_CALL AquaClipboard::removeClipboardListener(const Reference< XClipboardListener >& listener)
  throw( RuntimeException, std::exception )
{
  MutexGuard aGuard(m_aMutex);

  if (!listener.is())
     throw IllegalArgumentException("empty reference",
                                   static_cast<XClipboardEx*>(this), 1);

  mClipboardListeners.remove(listener);
}

void AquaClipboard::applicationDidBecomeActive(NSNotification*)
{
  ClearableMutexGuard aGuard(m_aMutex);

  int currentPboardChgCount = [mPasteboard changeCount];

  if (currentPboardChgCount != mPasteboardChangeCount)
    {
      mPasteboardChangeCount = currentPboardChgCount;

      // Clear clipboard content and owner and send lostOwnership
      // notification to the old clipboard owner as well as
      // ClipboardChanged notification to any clipboard listener
      Reference<XClipboardOwner> oldOwner(mXClipboardOwner);
      mXClipboardOwner.clear();

      Reference<XTransferable> oldContent(mXClipboardContent);
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
    ClearableMutexGuard aGuard(m_aMutex);

    list<Reference< XClipboardListener > > listeners(mClipboardListeners);
    ClipboardEvent aEvent;

    if (!listeners.empty())
    {
        aEvent = ClipboardEvent(static_cast<OWeakObject*>(this), getContents());
    }

    aGuard.clear();

    while (!listeners.empty())
    {
        if (listeners.front().is())
        {
#ifdef __OBJC__
            @try {
#else
            try {
#endif
                listeners.front()->changedContents(aEvent);
            }
#ifdef __OBJC__
            @catch (...) { }
#else
            catch (RuntimeException&) { }
#endif
        }
        listeners.pop_front();
    }
}

void AquaClipboard::fireLostClipboardOwnershipEvent(Reference<XClipboardOwner> const & oldOwner, Reference<XTransferable> const & oldContent)
{
    assert(oldOwner.is());
#ifdef __OBJC__
    @try {
#else
    try {
#endif
        oldOwner->lostOwnership( static_cast<XClipboardEx*>(this), oldContent );
    }
#ifdef __OBJC__
    @catch(...) { }
#else
    catch(RuntimeException&) { }
#endif
}

void AquaClipboard::provideDataForType(NSPasteboard* sender, const NSString* type)
{
    if( mXClipboardContent.is() )
    {
        DataProviderPtr_t dp = mpDataFlavorMapper->getDataProvider(type, mXClipboardContent);
        NSData* pBoardData = nullptr;

        if (dp.get() != nullptr)
        {
            pBoardData = (NSData*)dp->getSystemData();
            [sender setData: pBoardData forType:const_cast<NSString*>(type)];
        }
    }
}

void SAL_CALL AquaClipboard::flushClipboard()
  throw(RuntimeException, std::exception)
{
    if (mXClipboardContent.is())
    {
        Sequence<DataFlavor> flavorList = mXClipboardContent->getTransferDataFlavors();
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

OUString SAL_CALL AquaClipboard::getImplementationName() throw( RuntimeException, std::exception )
{
    return clipboard_getImplementationName();
}

sal_Bool SAL_CALL AquaClipboard::supportsService( const OUString& ServiceName ) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL AquaClipboard::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    return clipboard_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
