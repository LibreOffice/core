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

#ifndef INCLUDED_VCL_OSX_DRAGSOURCE_HXX
#define INCLUDED_VCL_OSX_DRAGSOURCE_HXX

#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/thread.h>
#include <com/sun/star/awt/MouseEvent.hpp>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

class DragSource;
class AquaSalFrame;

/* The functions declared in this protocol are actually
   declared in vcl/inc/osx/salframe.h. Because we want
   to avoid importing VCL headers in UNO services and
   on the other hand want to avoid warnings caused by
   gcc complaining about unknowness of these functions
   we declare them in a protocol here and cast at the
   appropriate places.
*/
@protocol MouseEventListener
-(void)registerMouseEventListener:(id)theHandler;
-(void)unregisterMouseEventListener:(id)theHandler;
@end

@interface DragSourceHelper : NSObject
{
  DragSource* mDragSource;
}

-(DragSourceHelper*)initWithDragSource: (DragSource*) pds;

-(void)mouseDown: (NSEvent*)theEvent;
-(void)mouseDragged: (NSEvent*)theEvent;

-(unsigned int)draggingSourceOperationMaskForLocal:(BOOL)isLocal;
-(void)draggedImage:(NSImage*)anImage beganAt:(NSPoint)aPoint;
-(void)draggedImage:(NSImage *)anImage endedAt:(NSPoint)aPoint operation:(NSDragOperation)operation;
-(void)draggedImage:(NSImage *)draggedImage movedTo:(NSPoint)screenPoint;

@end

class DragSource : public ::cppu::BaseMutex,
                   public ::cppu::WeakComponentImplHelper< css::datatransfer::dnd::XDragSource,
                                                            css::lang::XInitialization,
                                                            css::lang::XServiceInfo >
{
public:
  DragSource();
  virtual ~DragSource() override;
  DragSource(const DragSource&) = delete;
  DragSource& operator=(const DragSource&) = delete;

  // XInitialization
  virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
    SAL_THROW_IfNotObjectiveC( css::uno::Exception, std::exception/*, css::uno::RuntimeException*/ ) override;

  // XDragSource
  virtual sal_Bool SAL_CALL isDragImageSupported(  ) SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException, std::exception ) override;

  virtual sal_Int32 SAL_CALL getDefaultCursor(sal_Int8 dragAction)
    SAL_THROW_IfNotObjectiveC( css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;

  virtual void SAL_CALL startDrag( const css::datatransfer::dnd::DragGestureEvent& trigger,
                                   sal_Int8 sourceActions,
                                   sal_Int32 cursor,
                                   sal_Int32 image,
                                   const css::uno::Reference< css::datatransfer::XTransferable >& transferable,
                                   const css::uno::Reference< css::datatransfer::dnd::XDragSourceListener >& listener )
    SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException, std::exception ) override;

  // XServiceInfo
  virtual OUString SAL_CALL getImplementationName()
    SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException, std::exception ) override;
  virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
    SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException, std::exception ) override;
  virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
    SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException, std::exception ) override;

  void saveMouseEvent(NSEvent* theEvent);
  unsigned int getSupportedDragOperations(bool isLocal) const;

public:
  // The context notifies the XDragSourceListeners
  css::uno::Reference< css::datatransfer::dnd::XDragSourceContext > mXCurrentContext;

  id mView;
  AquaSalFrame* mpFrame;
  NSEvent* mLastMouseEventBeforeStartDrag;
  DragSourceHelper* mDragSourceHelper;
  css::awt::MouseEvent mMouseEvent;
  css::uno::Reference< css::datatransfer::XTransferable > mXTransferable;
  css::uno::Reference< css::datatransfer::dnd::XDragSourceListener > mXDragSrcListener;
  // The mouse button that set off the drag and drop operation
  short m_MouseButton;
  sal_Int8 mDragSourceActions;

  static css::uno::Reference< css::datatransfer::XTransferable > g_XTransferable;
  static NSView* g_DragSourceView;
  static bool    g_DropSuccessSet;
  static bool    g_DropSuccess;

};

#endif // INCLUDED_VCL_OSX_DRAGSOURCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
