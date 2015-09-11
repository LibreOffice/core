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

#include <boost/noncopyable.hpp>

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
                   public ::cppu::WeakComponentImplHelper< com::sun::star::datatransfer::dnd::XDragSource,
                                                            com::sun::star::lang::XInitialization,
                                                            com::sun::star::lang::XServiceInfo >,
                   private ::boost::noncopyable
{
public:
  DragSource();
  virtual ~DragSource();

  // XInitialization
  virtual void SAL_CALL initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
    throw(com::sun::star::uno::Exception, std::exception/*, com::sun::star::uno::RuntimeException*/) SAL_OVERRIDE;

  // XDragSource
  virtual sal_Bool SAL_CALL isDragImageSupported(  ) throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

  virtual sal_Int32 SAL_CALL getDefaultCursor(sal_Int8 dragAction)
    throw(com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

  virtual void SAL_CALL startDrag( const com::sun::star::datatransfer::dnd::DragGestureEvent& trigger,
                                   sal_Int8 sourceActions,
                                   sal_Int32 cursor,
                                   sal_Int32 image,
                                   const com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable >& transferable,
                                   const com::sun::star::uno::Reference< com::sun::star::datatransfer::dnd::XDragSourceListener >& listener )
    throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

  // XServiceInfo
  virtual OUString SAL_CALL getImplementationName() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
  virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
  virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

  void saveMouseEvent(NSEvent* theEvent);
  unsigned int getSupportedDragOperations(bool isLocal) const;

public:
  // The context notifies the XDragSourceListeners
  com::sun::star::uno::Reference< com::sun::star::datatransfer::dnd::XDragSourceContext > mXCurrentContext;

  id mView;
  AquaSalFrame* mpFrame;
  NSEvent* mLastMouseEventBeforeStartDrag;
  DragSourceHelper* mDragSourceHelper;
  com::sun::star::awt::MouseEvent mMouseEvent;
  com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable > mXTransferable;
  com::sun::star::uno::Reference< com::sun::star::datatransfer::dnd::XDragSourceListener > mXDragSrcListener;
  // The mouse button that set off the drag and drop operation
  short m_MouseButton;
  sal_Int8 mDragSourceActions;

  static com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable > g_XTransferable;
  static NSView* g_DragSourceView;
  static bool    g_DropSuccessSet;
  static bool    g_DropSuccess;

};

#endif // INCLUDED_VCL_OSX_DRAGSOURCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
