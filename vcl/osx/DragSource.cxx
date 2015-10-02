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

#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/awt/MouseButton.hpp>

#include "rtl/ustring.hxx"

#include <cppuhelper/supportsservice.hxx>

#include "DragSource.hxx"
#include "DragSourceContext.hxx"
#include "clipboard.hxx"
#include "DragActionConversion.hxx"

#include "osx/salframe.h"

#include <cassert>

using namespace cppu;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer::dnd::DNDConstants;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt::MouseButton;
using namespace com::sun::star::awt;
using namespace com::sun::star::lang;
using namespace comphelper;

// For LibreOffice internal D&D we provide the Transferable without NSDragPboard
// interference as a shortcut, see tdf#100097 for how dbaccess depends on this
uno::Reference<XTransferable> DragSource::g_XTransferable;
NSView* DragSource::g_DragSourceView = nil;
bool DragSource::g_DropSuccessSet = false;
bool DragSource::g_DropSuccess = false;

OUString dragSource_getImplementationName()
{
  return OUString("com.sun.star.comp.datatransfer.dnd.OleDragSource_V1");
}

Sequence<OUString> dragSource_getSupportedServiceNames()
{
  return { OUString("com.sun.star.datatransfer.dnd.OleDragSource") };
}

@implementation DragSourceHelper;

-(DragSourceHelper*)initWithDragSource: (DragSource*) pds
{
  self = [super init];

  if (self)
    {
      mDragSource = pds;
    }

  return self;
}

-(void)mouseDown: (NSEvent*)theEvent
{
  mDragSource->saveMouseEvent(theEvent);
}

-(void)mouseDragged: (NSEvent*)theEvent
{
  mDragSource->saveMouseEvent(theEvent);
}

-(unsigned int)draggingSourceOperationMaskForLocal: (BOOL)isLocal
{
  return mDragSource->getSupportedDragOperations(isLocal);
}

-(void)draggedImage:(NSImage*)anImage beganAt:(NSPoint)aPoint
{
    (void)anImage;
    (void)aPoint;
    DragSourceDragEvent dsde(static_cast<OWeakObject*>(mDragSource),
                             new DragSourceContext,
                             mDragSource,
                             DNDConstants::ACTION_COPY,
                             DNDConstants::ACTION_COPY);

    mDragSource->mXDragSrcListener->dragEnter(dsde);
}

-(void)draggedImage:(NSImage *)anImage endedAt:(NSPoint)aPoint operation:(NSDragOperation)operation
{
    (void)anImage;
    (void)aPoint;
    // an internal drop can accept the drop but fail with dropComplete( false )
    // this is different than the Cocoa API
    bool bDropSuccess = operation != NSDragOperationNone;
    if( DragSource::g_DropSuccessSet )
        bDropSuccess = DragSource::g_DropSuccess;

    DragSourceDropEvent dsde(static_cast<OWeakObject*>(mDragSource),
                             new DragSourceContext,
                             static_cast< XDragSource* >(mDragSource),
                             SystemToOfficeDragActions(operation),
                             bDropSuccess );

    mDragSource->mXDragSrcListener->dragDropEnd(dsde);
    mDragSource->mXDragSrcListener.clear();
}

-(void)draggedImage:(NSImage *)draggedImage movedTo:(NSPoint)screenPoint
{
    (void)draggedImage;
    (void)screenPoint;
    DragSourceDragEvent dsde(static_cast<OWeakObject*>(mDragSource),
                             new DragSourceContext,
                             mDragSource,
                             DNDConstants::ACTION_COPY,
                             DNDConstants::ACTION_COPY);

    mDragSource->mXDragSrcListener->dragOver(dsde);
}

@end

DragSource::DragSource():
  WeakComponentImplHelper<XDragSource, XInitialization, XServiceInfo>(m_aMutex),
  mView(nullptr),
  mpFrame(nullptr),
  mLastMouseEventBeforeStartDrag(nil),
  m_MouseButton(0)
{
}

DragSource::~DragSource()
{
    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
        [(id <MouseEventListener>)mView unregisterMouseEventListener: mDragSourceHelper];
    [mDragSourceHelper release];
}

void SAL_CALL DragSource::initialize(const Sequence< Any >& aArguments)
  SAL_THROW_IfNotObjectiveC( Exception, std::exception )
{
  if (aArguments.getLength() < 2)
    {
      #ifndef __OBJC__
      throw Exception("DragSource::initialize: Not enough parameter.",
                      static_cast<OWeakObject*>(this));
      #else
      return;
      #endif
    }

  Any pNSView = aArguments[1];
  sal_uInt64 tmp = 0;
  pNSView >>= tmp;
  mView = reinterpret_cast<NSView*>(tmp);

  /* All SalFrameView the base class for all VCL system views inherits from
     NSView in order to get mouse and other events. This is the only way to
     get these events. In order to start a drag operation we need to provide
     the mouse event which was the trigger. SalFrameView therefore implements
     a hook mechanism so that we can get mouse events for our purpose.
  */
  if (![mView respondsToSelector: @selector(registerMouseEventListener:)] ||
      ![mView respondsToSelector: @selector(unregisterMouseEventListener:)])
    {
      #ifndef __OBJC__
      throw Exception("DragSource::initialize: Provided view doesn't support mouse listener",
                      static_cast<OWeakObject*>(this));
      #else
      return;
      #endif
    }
  NSWindow* pWin = [mView window];
  if( ! pWin || ![pWin respondsToSelector: @selector(getSalFrame)] )
  {
      #ifndef __OBJC__
      throw Exception("DragSource::initialize: Provided view is not attached to a vcl frame",
                      static_cast<OWeakObject*>(this));
      #else
      return;
      #endif
  }
  mpFrame = reinterpret_cast<AquaSalFrame*>([pWin performSelector: @selector(getSalFrame)]);

  mDragSourceHelper = [[DragSourceHelper alloc] initWithDragSource: this];

  if (mDragSourceHelper == nil)
    {
      #ifndef __OBJC__
      throw Exception("DragSource::initialize: Cannot initialize DragSource",
                      static_cast<OWeakObject*>(this));
      #else
      return;
      #endif
    }

  [(id <MouseEventListener>)mView registerMouseEventListener: mDragSourceHelper];
}

sal_Bool SAL_CALL DragSource::isDragImageSupported(  )
  SAL_THROW_IfNotObjectiveC( RuntimeException, std::exception )
{
  return true;
}

sal_Int32 SAL_CALL DragSource::getDefaultCursor( sal_Int8 /*dragAction*/ )
  SAL_THROW_IfNotObjectiveC( IllegalArgumentException, RuntimeException, std::exception )
{
  return 0;
}

void SAL_CALL DragSource::startDrag(const DragGestureEvent& trigger,
                                    sal_Int8 sourceActions,
                                    sal_Int32 /*cursor*/,
                                    sal_Int32 /*image*/,
                                    const uno::Reference<XTransferable >& transferable,
                                    const uno::Reference<XDragSourceListener >& listener )
  SAL_THROW_IfNotObjectiveC( RuntimeException, std::exception )
{
  MutexGuard guard(m_aMutex);

  assert(listener.is() && "DragSource::startDrag: No XDragSourceListener provided\n");
  assert(transferable.is() && "DragSource::startDrag: No transferable provided\n");

  trigger.Event >>= mMouseEvent;
  m_MouseButton= mMouseEvent.Buttons;
  mXDragSrcListener = listener;
  mXCurrentContext = static_cast<XDragSourceContext*>(new DragSourceContext);
  rtl::Reference<AquaClipboard> clipb(new AquaClipboard(nullptr, false));
  g_XTransferable = transferable;
  clipb->setContents(g_XTransferable, uno::Reference<XClipboardOwner>());
  mDragSourceActions = sourceActions;
  g_DragSourceView = mView;

  NSSize sz;
  sz.width = 5;
  sz.height = 5;

  NSImage* dragImage;
  dragImage = [[NSImage alloc] initWithSize: sz];

  NSRect bounds;
  bounds.origin = NSMakePoint(0,0);
  bounds.size = sz;

  [dragImage lockFocus];
  [[NSColor blackColor] set];
  [NSBezierPath fillRect: bounds];
  [dragImage unlockFocus];

  NSPoint pInWnd = [mLastMouseEventBeforeStartDrag locationInWindow];
  NSPoint p;
  p = [mView convertPoint: pInWnd fromView: nil];
  p.x = p.x - sz.width/2;
  p.y = p.y - sz.height/2;

  // reset drop success flags
  g_DropSuccessSet = false;
  g_DropSuccess = false;

  SAL_WNODEPRECATED_DECLARATIONS_PUSH
    //TODO: 10.7 dragImage:at:offset:event:pasteboard:source:slideBack:
  [mView dragImage: dragImage
   at: p
   offset: NSMakeSize(0,0)
   event: mLastMouseEventBeforeStartDrag
   pasteboard: clipb->getPasteboard()
   source: mDragSourceHelper
   slideBack: 1];
   SAL_WNODEPRECATED_DECLARATIONS_POP

  [dragImage release];

  g_XTransferable.clear();
  g_DragSourceView = nil;

  // reset drop success flags
  g_DropSuccessSet = false;
  g_DropSuccess = false;
}

// In order to initiate a D&D operation we need to
// provide the triggering mouse event which we get
// from the SalFrameView that is associated with
// this DragSource
void DragSource::saveMouseEvent(NSEvent* theEvent)
{
  if (mLastMouseEventBeforeStartDrag != nil)
    {
      [mLastMouseEventBeforeStartDrag release];
    }

  mLastMouseEventBeforeStartDrag = theEvent;
}

/* isLocal indicates whether or not the DnD operation is OOo
   internal.
 */
unsigned int DragSource::getSupportedDragOperations(bool isLocal) const
{
  unsigned int srcActions = OfficeToSystemDragActions(mDragSourceActions);

  if (isLocal)
    {
      // Support NSDragOperation generic which means we can
      // decide which D&D operation to choose. We map
      // NSDragOperationGenric to DNDConstants::ACTION_DEFAULT
      // in SystemToOfficeDragActions to signal this and
      // use it in DropTarget::determineDropAction
      srcActions |= NSDragOperationGeneric;
    }
  else
    {
      // Mask out link and move operations on external DnD
      srcActions &= ~(NSDragOperationMove | NSDragOperationLink);
    }

  return srcActions;
}

OUString SAL_CALL DragSource::getImplementationName(  )
  SAL_THROW_IfNotObjectiveC( RuntimeException, std::exception )
{
  return dragSource_getImplementationName();
}

sal_Bool SAL_CALL DragSource::supportsService( const OUString& ServiceName )
  SAL_THROW_IfNotObjectiveC( RuntimeException, std::exception )
{
  return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL DragSource::getSupportedServiceNames()
  SAL_THROW_IfNotObjectiveC( RuntimeException, std::exception )
{
  return dragSource_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
