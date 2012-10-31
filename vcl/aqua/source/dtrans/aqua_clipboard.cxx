/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "aqua_clipboard.hxx"

#include "DataFlavorMapping.hxx"
#include "OSXTransferable.hxx"

#include "comphelper/makesequence.hxx"
#include "comphelper/processfactory.hxx"

#include <boost/assert.hpp>

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace std;
using namespace comphelper;

using ::rtl::OUString;

@implementation EventListener;

-(EventListener*)initWithAquaClipboard: (AquaClipboard*) pcb
{
    self = [super init];

    if (self)
        pAquaClipboard = pcb;

    return self;
}

-(void)pasteboard:(NSPasteboard*)sender provideDataForType:(NSString*)type
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
    pAquaClipboard = NULL;
}

@end


OUString clipboard_getImplementationName()
{
  return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.clipboard.AquaClipboard"));
}

Sequence<OUString> clipboard_getSupportedServiceNames()
{
  return makeSequence(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.clipboard.SystemClipboard")));
}


AquaClipboard::AquaClipboard(NSPasteboard* pasteboard, bool bUseSystemPasteboard) :
  WeakComponentImplHelper4<XClipboardEx, XClipboardNotifier, XFlushableClipboard, XServiceInfo>(m_aMutex),
  mIsSystemPasteboard(bUseSystemPasteboard)
{
    Reference<XMultiServiceFactory> mrServiceMgr = comphelper::getProcessServiceFactory();

    mrXMimeCntFactory = Reference<XMimeContentTypeFactory>(mrServiceMgr->createInstance(
     OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.MimeContentTypeFactory"))), UNO_QUERY);

  if (!mrXMimeCntFactory.is())
    {
      throw RuntimeException(OUString(
            RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot create com.sun.star.datatransfer.MimeContentTypeFactory")),
            static_cast<XClipboardEx*>(this));
    }

  mpDataFlavorMapper = DataFlavorMapperPtr_t(new DataFlavorMapper());

  if (pasteboard != NULL)
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
          throw RuntimeException(OUString(
                RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot create Cocoa pasteboard")),
                static_cast<XClipboardEx*>(this));
        }
    }

  [mPasteboard retain];

  mEventListener = [[EventListener alloc] initWithAquaClipboard: this];

  if (mEventListener == nil)
    {
      [mPasteboard release];

      throw RuntimeException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot create pasteboard change listener")),
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


Reference<XTransferable> SAL_CALL AquaClipboard::getContents() throw(RuntimeException)
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
        throw( RuntimeException )
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


OUString SAL_CALL AquaClipboard::getName() throw( RuntimeException )
{
  return OUString();
}


sal_Int8 SAL_CALL AquaClipboard::getRenderingCapabilities() throw( RuntimeException )
{
    return 0;
}


void SAL_CALL AquaClipboard::addClipboardListener(const Reference< XClipboardListener >& listener)
  throw( RuntimeException )
{
  MutexGuard aGuard(m_aMutex);

  if (!listener.is())
     throw IllegalArgumentException(OUString(RTL_CONSTASCII_USTRINGPARAM("empty reference")),
                                   static_cast<XClipboardEx*>(this), 1);

  mClipboardListeners.push_back(listener);
}


void SAL_CALL AquaClipboard::removeClipboardListener(const Reference< XClipboardListener >& listener)
  throw( RuntimeException )
{
  MutexGuard aGuard(m_aMutex);

  if (!listener.is())
     throw IllegalArgumentException(OUString(RTL_CONSTASCII_USTRINGPARAM("empty reference")),
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
      mXClipboardOwner = Reference<XClipboardOwner>();

      Reference<XTransferable> oldContent(mXClipboardContent);
      mXClipboardContent = Reference<XTransferable>();

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

    if (listeners.begin() != listeners.end())
      {
        aEvent = ClipboardEvent(static_cast<OWeakObject*>(this), getContents());
      }

    aGuard.clear();

    while (listeners.begin() != listeners.end())
    {
        if (listeners.front().is())
          {
            try { listeners.front()->changedContents(aEvent); }
            catch (RuntimeException&) { }
          }
        listeners.pop_front();
    }
}


void AquaClipboard::fireLostClipboardOwnershipEvent(Reference<XClipboardOwner> oldOwner, Reference<XTransferable> oldContent)
{
  BOOST_ASSERT(oldOwner.is());

  try { oldOwner->lostOwnership(static_cast<XClipboardEx*>(this), oldContent); }
  catch(RuntimeException&) { }
}


void AquaClipboard::provideDataForType(NSPasteboard* sender, NSString* type)
{
    if( mXClipboardContent.is() )
    {
        DataProviderPtr_t dp = mpDataFlavorMapper->getDataProvider(type, mXClipboardContent);
        NSData* pBoardData = NULL;

        if (dp.get() != NULL)
        {
            pBoardData = (NSData*)dp->getSystemData();
            [sender setData: pBoardData forType: type];
        }
    }
}


//------------------------------------------------
// XFlushableClipboard
//------------------------------------------------

void SAL_CALL AquaClipboard::flushClipboard()
  throw(RuntimeException)
{
    if (mXClipboardContent.is())
    {
          Sequence<DataFlavor> flavorList = mXClipboardContent->getTransferDataFlavors();
        sal_uInt32 nFlavors = flavorList.getLength();

        for (sal_uInt32 i = 0; i < nFlavors; i++)
        {
            NSString* sysType = mpDataFlavorMapper->openOfficeToSystemFlavor(flavorList[i]);

            if (sysType != NULL)
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


//-------------------------------------------------
// XServiceInfo
//-------------------------------------------------

OUString SAL_CALL AquaClipboard::getImplementationName() throw( RuntimeException )
{
  return clipboard_getImplementationName();
}


sal_Bool SAL_CALL AquaClipboard::supportsService( const OUString& /*ServiceName*/ ) throw( RuntimeException )
{
    return sal_False;
}


Sequence< OUString > SAL_CALL AquaClipboard::getSupportedServiceNames() throw( RuntimeException )
{
  return clipboard_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
