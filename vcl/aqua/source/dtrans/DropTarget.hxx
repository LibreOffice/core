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

#ifndef _DROPTARGET_HXX_
#define _DROPTARGET_HXX_

#include "DataFlavorMapping.hxx"
#include <cppuhelper/compbase5.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>

#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#include <com/sun/star/datatransfer/dnd/DropTargetDragEnterEvent.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDropContext.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <boost/utility.hpp>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

class DropTarget;
class AquaSalFrame;

/* The functions declared in this protocol are actually
   declared in vcl/aqua/inc/salframe.h. Because we want
   to avoid importing VCL headers in UNO services and
   on the other hand want to avoid warnings caused by
   gcc complaining about unknowness of these functions
   we declare them in a protocol here and cast at the
   appropriate places.
*/
@protocol DraggingDestinationHandler
-(void)registerDraggingDestinationHandler:(id)theHandler;
-(void)unregisterDraggingDestinationHandler:(id)theHandler;
@end


@interface DropTargetHelper : NSObject
{
  DropTarget* mDropTarget;
}

-(DropTargetHelper*)initWithDropTarget:(DropTarget*)pdt;

-(NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender;
-(NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender;
-(void)draggingExited:(id <NSDraggingInfo>)sender;
-(BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender;
-(BOOL)performDragOperation:(id <NSDraggingInfo>)sender;
-(void)concludeDragOperation:(id <NSDraggingInfo>)sender;

@end


class DropTarget: public cppu::BaseMutex,
                  public cppu::WeakComponentImplHelper5< com::sun::star::lang::XInitialization,
                                                         com::sun::star::datatransfer::dnd::XDropTarget,
                                                         com::sun::star::datatransfer::dnd::XDropTargetDragContext,
                                                         com::sun::star::datatransfer::dnd::XDropTargetDropContext,
                                                         com::sun::star::lang::XServiceInfo >,
                  private boost::noncopyable
{
public:
  DropTarget();
  virtual ~DropTarget();

  // Overrides WeakComponentImplHelper::disposing which is called by
  // WeakComponentImplHelper::dispose
  // Must be called.
  virtual void SAL_CALL disposing();

  // XInitialization
  virtual void SAL_CALL initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
    throw(com::sun::star::uno::Exception);

  // XDropTarget
  virtual void SAL_CALL addDropTargetListener( const com::sun::star::uno::Reference< com::sun::star::datatransfer::dnd::XDropTargetListener >& dtl )
    throw(com::sun::star::uno::RuntimeException);

  virtual void SAL_CALL removeDropTargetListener( const com::sun::star::uno::Reference<  com::sun::star::datatransfer::dnd::XDropTargetListener >& dtl )
    throw(com::sun::star::uno::RuntimeException);

  // Default is not active
  virtual sal_Bool SAL_CALL isActive() throw(com::sun::star::uno::RuntimeException);
  virtual void SAL_CALL setActive(sal_Bool isActive) throw(com::sun::star::uno::RuntimeException);
  virtual sal_Int8 SAL_CALL getDefaultActions() throw(com::sun::star::uno::RuntimeException);
  virtual void SAL_CALL setDefaultActions(sal_Int8 actions) throw(com::sun::star::uno::RuntimeException);

  // XDropTargetDragContext
  virtual void SAL_CALL acceptDrag(sal_Int8 dragOperation) throw(com::sun::star::uno::RuntimeException);
  virtual void SAL_CALL rejectDrag() throw(com::sun::star::uno::RuntimeException);

  // XDropTargetDragContext
  virtual void SAL_CALL acceptDrop(sal_Int8 dropOperation) throw (com::sun::star::uno::RuntimeException);
  virtual void SAL_CALL rejectDrop() throw (com::sun::star::uno::RuntimeException);
  virtual void SAL_CALL dropComplete(sal_Bool success) throw (com::sun::star::uno::RuntimeException);

  // XServiceInfo
  virtual rtl::OUString SAL_CALL getImplementationName() throw (com::sun::star::uno::RuntimeException);
  virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw (com::sun::star::uno::RuntimeException);
  virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException);

  // NSDraggingDestination protocol functions
  virtual NSDragOperation draggingEntered(id sender);
  virtual NSDragOperation draggingUpdated(id sender);
  virtual void draggingExited(id sender);
  virtual BOOL prepareForDragOperation(id sender);
  virtual BOOL performDragOperation(id sender);
  virtual void concludeDragOperation(id sender);

  /* If multiple actions are supported by the drag source and
     the user did not choose a specific action by pressing a
     modifier key choose a default action to be proposed to
     the application.
  */
  sal_Int8 determineDropAction(sal_Int8 dropActions, id sender) const;

private:
  void fire_drop(const com::sun::star::datatransfer::dnd::DropTargetDropEvent& dte);
  void fire_dragEnter(const com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& dtdee);
  void fire_dragExit(const com::sun::star::datatransfer::dnd::DropTargetEvent& dte);
  void fire_dragOver(const com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde);
  void fire_dropActionChanged(const com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde);

private:
  com::sun::star::uno::Reference< com::sun::star::datatransfer::dnd::XDropTargetDragContext > mXCurrentDragContext;
  com::sun::star::uno::Reference< com::sun::star::datatransfer::dnd::XDropTargetDropContext > mXCurrentDropContext;
  com::sun::star::uno::Reference< com::sun::star::datatransfer::clipboard::XClipboard > mXCurrentDragClipboard;
  DataFlavorMapperPtr_t mDataFlavorMapper;
  id  mView;
  AquaSalFrame* mpFrame;
  DropTargetHelper* mDropTargetHelper;
  bool mbActive;
  sal_Int8 mDragSourceSupportedActions;
  sal_Int8 mSelectedDropAction;
  sal_Int8 mDefaultActions;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
