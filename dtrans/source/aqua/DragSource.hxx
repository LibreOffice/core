/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DragSource.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 14:47:11 $
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

#ifndef _DRAGSOURCE_HXX_
#define _DRAGSOURCE_HXX_

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGSOURCE_HPP_
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGSOURCECONTEXT_HPP_
#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _CPPUHELPER_BASEMUTEX_HXX_
#include <cppuhelper/basemutex.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _OSL_INTERLOCK_H_
#include <interlck.h>
#endif

#ifndef _COM_SUN_STAR_AWT_MOUSEEVENT_HPP_
#include <com/sun/star/awt/MouseEvent.hpp>
#endif

#include <boost/utility.hpp>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>


class DragSource;

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

-(unsigned int)draggingSourceOperationMaskForLocal:(MacOSBOOL)isLocal;
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
  DragSource(const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext>& context);
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
  com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext> mXComponentContext;

  // The context notifies the XDragSourceListeners
  com::sun::star::uno::Reference< com::sun::star::datatransfer::dnd::XDragSourceContext > mXCurrentContext;

  id mView;
  NSEvent* mLastMouseEventBeforeStartDrag;
  DragSourceHelper* mDragSourceHelper;
  com::sun::star::awt::MouseEvent mMouseEvent;
  com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable > mXTransferable;
  com::sun::star::uno::Reference< com::sun::star::datatransfer::dnd::XDragSourceListener > mXDragSrcListener;
  // The mouse button that set off the drag and drop operation
  short m_MouseButton;
  sal_Int8 mDragSourceActions;
};


#endif
