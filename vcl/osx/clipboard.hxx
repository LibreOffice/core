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

#ifndef INCLUDED_VCL_OSX_CLIPBOARD_HXX
#define INCLUDED_VCL_OSX_CLIPBOARD_HXX

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

#include <vector>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

class AquaClipboard;

@interface EventListener : NSObject
{
     AquaClipboard* pAquaClipboard;
}

// Init the pasteboard change listener with a reference to the OfficeClipboard
// instance
- (EventListener*)initWithAquaClipboard: (AquaClipboard*) pcb;

// Promise resolver function
- (void)pasteboard:(NSPasteboard*)sender provideDataForType:(const NSString *)type;

-(void)applicationDidBecomeActive:(NSNotification*)aNotification;

-(void)disposing;
@end

class AquaClipboard : public ::cppu::BaseMutex,
                      public ::cppu::WeakComponentImplHelper<css::datatransfer::clipboard::XSystemClipboard,
                                                             css::datatransfer::clipboard::XFlushableClipboard,
                                                             css::lang::XServiceInfo>
{
public:
  /* Create a clipboard instance.

     @param pasteboard
     If not equal NULL the instance will be instantiated with the provided
     pasteboard reference and 'bUseSystemClipboard' will be ignored

     @param bUseSystemClipboard
     If 'pasteboard' is NULL 'bUseSystemClipboard' determines whether the
     system clipboard will be created (bUseSystemClipboard == true) or if
     the DragPasteboard if bUseSystemClipboard == false
   */
  AquaClipboard(NSPasteboard* pasteboard,
                bool bUseSystemClipboard);

  virtual ~AquaClipboard() override;
  AquaClipboard(const AquaClipboard&) = delete;
  AquaClipboard& operator=(const AquaClipboard&) = delete;

  // XClipboard

  virtual css::uno::Reference<css::datatransfer::XTransferable> SAL_CALL getContents() override;

  virtual void SAL_CALL setContents(css::uno::Reference<css::datatransfer::XTransferable> const & xTransferable,
                                    css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner> const & xClipboardOwner) override;

  virtual OUString SAL_CALL getName() override;

  // XClipboardEx

  virtual sal_Int8 SAL_CALL getRenderingCapabilities() override;

  // XClipboardNotifier

  virtual void SAL_CALL addClipboardListener(css::uno::Reference<css::datatransfer::clipboard::XClipboardListener> const & listener) override;
  virtual void SAL_CALL removeClipboardListener(css::uno::Reference<css::datatransfer::clipboard::XClipboardListener> const & listener) override;

  // XFlushableClipboard

  virtual void SAL_CALL flushClipboard() override;

  // XServiceInfo

  virtual OUString SAL_CALL getImplementationName() override;
  virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
  virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

  /* Get a reference to the used pastboard.
   */
  NSPasteboard* getPasteboard() const;

  /* Notify the current clipboard owner that he is no longer the clipboard owner.
   */
  void fireLostClipboardOwnershipEvent(css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner> const & oldOwner,
                                       css::uno::Reference<css::datatransfer::XTransferable> const & oldContent);

  void pasteboardChangedOwner();

  void provideDataForType(NSPasteboard* sender, const NSString* type);

  void applicationDidBecomeActive(NSNotification* aNotification);

private:

  /* Notify all registered XClipboardListener that the clipboard content
     has changed.
  */
  void fireClipboardChangedEvent();

private:
  css::uno::Reference<css::datatransfer::XMimeContentTypeFactory> mrXMimeCntFactory;
  std::vector<css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>> mClipboardListeners;
  css::uno::Reference<css::datatransfer::XTransferable> mXClipboardContent;
  css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner> mXClipboardOwner;
  DataFlavorMapperPtr_t mpDataFlavorMapper;
  bool mIsSystemPasteboard;
  NSPasteboard* mPasteboard;
  int mPasteboardChangeCount;
  EventListener* mEventListener;
};

#endif // INCLUDED_VCL_OSX_CLIPBOARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
