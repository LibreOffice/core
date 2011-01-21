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

#ifndef _DRAGSOURCE_HXX_
#define _DRAGSOURCE_HXX_

#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/compbase3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/thread.h>
#include <com/sun/star/awt/MouseEvent.hpp>

#include <boost/utility.hpp>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>


class DragSource;
class AquaSalFrame;

/* The functions declared in this protocol are actually
   declared in vcl/aqua/inc/salframe.h. Because we want
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
                   public ::cppu::WeakComponentImplHelper3< com::sun::star::datatransfer::dnd::XDragSource,
                                                            com::sun::star::lang::XInitialization,
                                                            com::sun::star::lang::XServiceInfo >,
                   private ::boost::noncopyable
{
public:
  DragSource();
  virtual ~DragSource();

  // XInitialization
  virtual void SAL_CALL initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
    throw(com::sun::star::uno::Exception/*, com::sun::star::uno::RuntimeException*/);

  // XDragSource
  virtual sal_Bool SAL_CALL isDragImageSupported(  ) throw(com::sun::star::uno::RuntimeException);

  virtual sal_Int32 SAL_CALL getDefaultCursor(sal_Int8 dragAction)
    throw(com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException);

  virtual void SAL_CALL startDrag( const com::sun::star::datatransfer::dnd::DragGestureEvent& trigger,
                                   sal_Int8 sourceActions,
                                   sal_Int32 cursor,
                                   sal_Int32 image,
                                   const com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable >& transferable,
                                   const com::sun::star::uno::Reference< com::sun::star::datatransfer::dnd::XDragSourceListener >& listener )
    throw(com::sun::star::uno::RuntimeException);

  // XServiceInfo
  virtual rtl::OUString SAL_CALL getImplementationName() throw (com::sun::star::uno::RuntimeException);
  virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw (com::sun::star::uno::RuntimeException);
  virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException);

  virtual void saveMouseEvent(NSEvent* theEvent);
  virtual unsigned int getSupportedDragOperations(bool isLocal) const;

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


#endif
