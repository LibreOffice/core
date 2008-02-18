/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DropTarget.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 14:48:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _DROPTARGET_HXX_
#define _DROPTARGET_HXX_

#ifndef INCLUDED_DATAFLAVORMAPPING_HXX_
#include "DataFlavorMapping.hxx"
#endif

#ifndef _CPPUHELPER_COMPBASE5_HXX_
#include <cppuhelper/compbase5.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGET_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGETLISTENR_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_DROPTARGETDRAGENTEREVENT_HPP_
#include <com/sun/star/datatransfer/dnd/DropTargetDragEnterEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGETDRAGCONTEXT_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGETDROPCONTEXT_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTargetDropContext.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _CPPUHELPER_BASEMUTEX_HXX_
#include <cppuhelper/basemutex.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTICOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#endif

#include <boost/utility.hpp>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

class DropTarget;

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
-(MacOSBOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender;
-(MacOSBOOL)performDragOperation:(id <NSDraggingInfo>)sender;
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
  DropTarget(const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& context);
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
  virtual MacOSBOOL prepareForDragOperation(id sender);
  virtual MacOSBOOL performDragOperation(id sender);
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
  com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > mXComponentContext;
  com::sun::star::uno::Reference< com::sun::star::datatransfer::dnd::XDropTargetDragContext > mXCurrentDragContext;
  com::sun::star::uno::Reference< com::sun::star::datatransfer::dnd::XDropTargetDropContext > mXCurrentDropContext;
  com::sun::star::uno::Reference< com::sun::star::datatransfer::clipboard::XClipboard > mXCurrentDragClipboard;
  DataFlavorMapperPtr_t mDataFlavorMapper;
  id  mView;
  DropTargetHelper* mDropTargetHelper;
  bool mbActive;
  sal_Int8 mDragSourceSupportedActions;
  sal_Int8 mSelectedDropAction;
  sal_Int8 mDefaultActions;
};

#endif
