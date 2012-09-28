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


#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/awt/MouseButton.hpp>

#include "rtl/unload.h"
#include "rtl/ustring.hxx"

#include "comphelper/makesequence.hxx"

#include "DragSource.hxx"
#include "DragSourceContext.hxx"
#include "aqua_clipboard.hxx"
#include "DragActionConversion.hxx"

#include "aqua/salframe.h"

#include <memory>


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
using namespace std;

using ::rtl::OUString;

// For OOo internal D&D we provide the Transferable without NSDragPboard
// interference as a shortcut
uno::Reference<XTransferable> DragSource::g_XTransferable;
NSView* DragSource::g_DragSourceView = nil;
bool DragSource::g_DropSuccessSet = false;
bool DragSource::g_DropSuccess = false;


OUString dragSource_getImplementationName()
{
  return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.datatransfer.dnd.OleDragSource_V1"));
}

Sequence<OUString> dragSource_getSupportedServiceNames()
{
  return makeSequence(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.dnd.OleDragSource")));
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
    mDragSource->mXDragSrcListener = uno::Reference<XDragSourceListener>();
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
  WeakComponentImplHelper3<XDragSource, XInitialization, XServiceInfo>(m_aMutex),
  mView(NULL),
  mpFrame(NULL),
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
  throw(Exception)
{
  if (aArguments.getLength() < 2)
    {
      throw Exception(OUString(RTL_CONSTASCII_USTRINGPARAM("DragSource::initialize: Not enough parameter.")),
                      static_cast<OWeakObject*>(this));
    }

  Any pNSView = aArguments[1];
  sal_uInt64 tmp = 0;
  pNSView >>= tmp;
  mView = (NSView*)tmp;

  /* All SalFrameView the base class for all VCL system views inherits from
     NSView in order to get mouse and other events. This is the only way to
     get these events. In order to start a drag operation we need to provide
     the mouse event which was the trigger. SalFrameView therefor implements
     a hook mechanism so that we can get mouse events for our purpose.
  */
  if (![mView respondsToSelector: @selector(registerMouseEventListener:)] ||
      ![mView respondsToSelector: @selector(unregisterMouseEventListener:)])
    {
      throw Exception(OUString(RTL_CONSTASCII_USTRINGPARAM("DragSource::initialize: Provided view doesn't support mouse listener")),
                      static_cast<OWeakObject*>(this));
    }
  NSWindow* pWin = [mView window];
  if( ! pWin || ![pWin respondsToSelector: @selector(getSalFrame)] )
  {
      throw Exception(OUString(RTL_CONSTASCII_USTRINGPARAM("DragSource::initialize: Provided view is not attached to a vcl frame")),
                      static_cast<OWeakObject*>(this));
  }
  mpFrame = (AquaSalFrame*)[pWin performSelector: @selector(getSalFrame)];

  mDragSourceHelper = [[DragSourceHelper alloc] initWithDragSource: this];

  if (mDragSourceHelper == nil)
    {
      throw Exception(OUString(RTL_CONSTASCII_USTRINGPARAM("DragSource::initialize: Cannot initialize DragSource")),
                      static_cast<OWeakObject*>(this));
    }

  [(id <MouseEventListener>)mView registerMouseEventListener: mDragSourceHelper];
}


//----------------------------------------------------
// XDragSource
//----------------------------------------------------

sal_Bool SAL_CALL DragSource::isDragImageSupported(  )
  throw(RuntimeException)
{
  return true;
}


sal_Int32 SAL_CALL DragSource::getDefaultCursor( sal_Int8 /*dragAction*/ )
  throw( IllegalArgumentException, RuntimeException)
{
  return 0;
}


void SAL_CALL DragSource::startDrag(const DragGestureEvent& trigger,
                                    sal_Int8 sourceActions,
                                    sal_Int32 /*cursor*/,
                                    sal_Int32 /*image*/,
                                    const uno::Reference<XTransferable >& transferable,
                                    const uno::Reference<XDragSourceListener >& listener )
  throw( RuntimeException)
{
  MutexGuard guard(m_aMutex);

  OSL_ASSERT(listener.is() && "DragSource::startDrag: No XDragSourceListener provided\n");
  OSL_ASSERT(transferable.is() && "DragSource::startDrag: No transferable provided\n");

  trigger.Event >>= mMouseEvent;
  m_MouseButton= mMouseEvent.Buttons;
  mXDragSrcListener = listener;
  mXCurrentContext = static_cast<XDragSourceContext*>(new DragSourceContext);
  auto_ptr<AquaClipboard> clipb(new AquaClipboard(NULL, false));
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

  [mView dragImage: dragImage
   at: p
   offset: NSMakeSize(0,0)
   event: mLastMouseEventBeforeStartDrag
   pasteboard: clipb->getPasteboard()
   source: mDragSourceHelper
   slideBack: 1];

  [dragImage release];

  g_XTransferable = uno::Reference<XTransferable>();
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


//################################
// XServiceInfo
//################################

OUString SAL_CALL DragSource::getImplementationName(  ) throw (RuntimeException)
{
  return dragSource_getImplementationName();
}


sal_Bool SAL_CALL DragSource::supportsService( const OUString& ServiceName ) throw (RuntimeException)
{
  return ServiceName == "com.sun.star.datatransfer.dnd.OleDragSource";
}


Sequence< OUString > SAL_CALL DragSource::getSupportedServiceNames() throw (RuntimeException)
{
  return dragSource_getSupportedServiceNames();
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
