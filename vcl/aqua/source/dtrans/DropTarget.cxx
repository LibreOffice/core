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
#include <com/sun/star/datatransfer/dnd/DropTargetDragEnterEvent.hpp>
#include <rtl/unload.h>

#include "comphelper/makesequence.hxx"
#include <cppuhelper/interfacecontainer.hxx>

#include "aqua_clipboard.hxx"
#include "DropTarget.hxx"
#include "DragActionConversion.hxx"

#include "DragSource.hxx"

#include <rtl/ustring.h>
#include <stdio.h>

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#include <aqua/salframe.h>
#include <aqua/salframeview.h>

using namespace cppu;
using namespace osl;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer::dnd::DNDConstants;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star;
using namespace comphelper;

using ::rtl::OUString;

OUString dropTarget_getImplementationName()
{
  return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.datatransfer.dnd.OleDropTarget_V1"));
}


Sequence<OUString> dropTarget_getSupportedServiceNames()
{
  return makeSequence(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.dnd.OleDropTarget")));
}


namespace /* private */
{
  // Cocoa's coordinate system has its origin lower-left, VCL's
  // coordinate system upper-left hence we need to transform
  // coordinates

  inline void CocoaToVCL(NSPoint& rPoint, const NSRect& bounds)
  {
    rPoint.y = bounds.size.height - rPoint.y;
  }
}


@implementation DropTargetHelper


-(DropTargetHelper*)initWithDropTarget:(DropTarget*)pdt
{
  self = [super init];

  if (self)
    {
      mDropTarget = pdt;
    }

  return self;
}


-(NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
  return mDropTarget->draggingEntered(sender);
}


-(NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
  return mDropTarget->draggingUpdated(sender);
}


-(void)draggingExited:(id <NSDraggingInfo>)sender
{
  mDropTarget->draggingExited(sender);
}


-(BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender
{
  return mDropTarget->prepareForDragOperation(sender);
}


-(BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
  return mDropTarget->performDragOperation(sender);
}


-(void)concludeDragOperation:(id <NSDraggingInfo>)sender
{
  mDropTarget->concludeDragOperation(sender);
}


@end


DropTarget::DropTarget() :
  WeakComponentImplHelper5<XInitialization, XDropTarget, XDropTargetDragContext, XDropTargetDropContext, XServiceInfo>(m_aMutex),
  mView(nil),
  mpFrame(NULL),
  mDropTargetHelper(nil),
  mbActive(false),
  mDragSourceSupportedActions(DNDConstants::ACTION_NONE),
  mSelectedDropAction(DNDConstants::ACTION_NONE),
  mDefaultActions(DNDConstants::ACTION_COPY_OR_MOVE | DNDConstants::ACTION_LINK | DNDConstants::ACTION_DEFAULT)
{
  mDataFlavorMapper = DataFlavorMapperPtr_t(new DataFlavorMapper());
}


DropTarget::~DropTarget()
{
    if( AquaSalFrame::isAlive( mpFrame ) )
        [(id <DraggingDestinationHandler>)mView unregisterDraggingDestinationHandler:mDropTargetHelper];
    [mDropTargetHelper release];
}


sal_Int8 DropTarget::determineDropAction(sal_Int8 dropActions, id sender) const
{
  sal_Int8 dropAct = dropActions;
  bool srcAndDestEqual = false;

  if ([sender draggingSource] != nil)
    {
      // Internal DnD
      NSView* destView = [[sender draggingDestinationWindow] contentView];
      srcAndDestEqual = (DragSource::g_DragSourceView == destView);
    }

  // If ACTION_DEFAULT is set this means NSDragOperationGeneric
  // has been set and we map this to ACTION_MOVE or ACTION_COPY
  // depending on whether or not source and dest are equal,
  // this hopefully satisfies all parties
  if( (dropActions == DNDConstants::ACTION_DEFAULT)
  || ((dropActions == mDragSourceSupportedActions)
     && !(~mDragSourceSupportedActions & DNDConstants::ACTION_COPY_OR_MOVE ) ) )
    {
      dropAct = srcAndDestEqual ? DNDConstants::ACTION_MOVE :
        DNDConstants::ACTION_COPY;
    }
     // if more than one drop actions have been specified
     // set ACTION_DEFAULT in order to let the drop target
     // decide which one to use
  else if (dropActions != DNDConstants::ACTION_NONE &&
           dropActions != DNDConstants::ACTION_MOVE &&
           dropActions != DNDConstants::ACTION_COPY &&
           dropActions != DNDConstants::ACTION_LINK)
    {
      if (srcAndDestEqual)
        {
          dropAct = dropActions;
        }
      else // source and destination are different
        {
          if (dropActions & DNDConstants::ACTION_COPY)
            dropAct = DNDConstants::ACTION_COPY;
          else if (dropActions & DNDConstants::ACTION_MOVE)
            dropAct = DNDConstants::ACTION_MOVE;
          else if (dropActions & DNDConstants::ACTION_LINK)
            dropAct = DNDConstants::ACTION_LINK;
        }

      dropAct |= DNDConstants::ACTION_DEFAULT;
    }

  return dropAct;
}


NSDragOperation DropTarget::draggingEntered(id sender)
{
  // Initially when DnD will be started no modifier key can be pressed yet
  // thus we are getting all actions that the drag source supports, we save
  // this value because later the system masks the drag source actions if
  // a modifier key will be pressed
  mDragSourceSupportedActions = SystemToOfficeDragActions([sender draggingSourceOperationMask]);

  // Only if the drop target is really interessted in the drag actions
  // supported by the source
  if (mDragSourceSupportedActions & mDefaultActions)
    {
      sal_Int8 currentAction = determineDropAction(mDragSourceSupportedActions, sender);

      NSRect bounds = [mView bounds];
      NSPoint dragLocation = [sender draggedImageLocation];

      CocoaToVCL(dragLocation, bounds);

      sal_Int32 posX = static_cast<sal_Int32>(dragLocation.x);
      sal_Int32 posY = static_cast<sal_Int32>(dragLocation.y);

      NSPasteboard* dragPboard = [sender draggingPasteboard];
      mXCurrentDragClipboard = new AquaClipboard(dragPboard, false);

      uno::Reference<XTransferable> xTransferable = DragSource::g_XTransferable.is() ?
        DragSource::g_XTransferable : mXCurrentDragClipboard->getContents();

      DropTargetDragEnterEvent dtdee(static_cast<OWeakObject*>(this),
                                     0,
                                     this,
                                     currentAction,
                                     posX,
                                     posY,
                                     mDragSourceSupportedActions,
                                     xTransferable->getTransferDataFlavors());

      fire_dragEnter(dtdee);
    }

  return OfficeToSystemDragActions(mSelectedDropAction);
}


NSDragOperation DropTarget::draggingUpdated(id sender)
{
  sal_Int8 currentDragSourceActions =
    SystemToOfficeDragActions([sender draggingSourceOperationMask]);
  NSDragOperation dragOp = NSDragOperationNone;

  if (currentDragSourceActions & mDefaultActions)
    {
      sal_Int8 currentAction = determineDropAction(currentDragSourceActions, sender);
      NSRect bounds = [mView bounds];
      NSPoint dragLocation = [sender draggedImageLocation];

      CocoaToVCL(dragLocation, bounds);

      sal_Int32 posX = static_cast<sal_Int32>(dragLocation.x);
      sal_Int32 posY = static_cast<sal_Int32>(dragLocation.y);

      DropTargetDragEvent dtde(static_cast<OWeakObject*>(this),
                               0,
                               this,
                               currentAction,
                               posX,
                               posY,
                               mDragSourceSupportedActions);

      fire_dragOver(dtde);

      // drag over callbacks likely have rendered something
      [mView setNeedsDisplay: TRUE];

      dragOp = OfficeToSystemDragActions(mSelectedDropAction);

      //NSLog(@"Drag update: Source actions: %x proposed action %x selected action %x", mDragSourceSupportedActions, currentAction, mSelectedDropAction);
    }

#ifndef __LP64__
  // Weird but it appears as if there is no method in Cocoa
  // to create a kThemeCopyArrowCursor hence we have to use
  // Carbon to do it
  if (dragOp == NSDragOperationNone)
    SetThemeCursor(kThemeNotAllowedCursor);
  else if (dragOp == NSDragOperationCopy)
    SetThemeCursor(kThemeCopyArrowCursor);
  else
    SetThemeCursor(kThemeArrowCursor);
#else
  // FIXME: SetThemeCursor replacement?
#endif
  return dragOp;
}


void DropTarget::draggingExited(id /*sender*/)
{
    DropTargetEvent dte(static_cast<OWeakObject*>(this), 0);
    fire_dragExit(dte);
    mDragSourceSupportedActions = DNDConstants::ACTION_NONE;
    mSelectedDropAction = DNDConstants::ACTION_NONE;
#ifndef __LP64__
    SetThemeCursor(kThemeArrowCursor);
#endif
}


BOOL DropTarget::prepareForDragOperation(id /*sender*/)
{
    return 1;
}


BOOL DropTarget::performDragOperation(id sender)
{
  bool bSuccess = false;

  if (mSelectedDropAction != DNDConstants::ACTION_NONE)
    {
        uno::Reference<XTransferable> xTransferable = DragSource::g_XTransferable;

      if (!DragSource::g_XTransferable.is())
        {
          xTransferable = mXCurrentDragClipboard->getContents();
        }

      NSRect bounds = [mView bounds];
      NSPoint dragLocation = [sender draggedImageLocation];

      CocoaToVCL(dragLocation, bounds);

      sal_Int32 posX = static_cast<sal_Int32>(dragLocation.x);
      sal_Int32 posY = static_cast<sal_Int32>(dragLocation.y);

      DropTargetDropEvent dtde(static_cast<OWeakObject*>(this),
                               0,
                               this,
                               mSelectedDropAction,
                               posX,
                               posY,
                               mDragSourceSupportedActions,
                               xTransferable);

      fire_drop(dtde);

      bSuccess = true;
    }

  return bSuccess;
}


void DropTarget::concludeDragOperation(id /*sender*/)
{
    mDragSourceSupportedActions = DNDConstants::ACTION_NONE;
    mSelectedDropAction = DNDConstants::ACTION_NONE;
    mXCurrentDragClipboard = uno::Reference<XClipboard>();
#ifndef __LP64__
    SetThemeCursor(kThemeArrowCursor);
#endif
}


  // called from WeakComponentImplHelperX::dispose
  // WeakComponentImplHelper calls disposing before it destroys
  // itself.
  void SAL_CALL DropTarget::disposing()
  {
  }


  void SAL_CALL DropTarget::initialize(const Sequence< Any >& aArguments)
    throw(Exception)
  {
    if (aArguments.getLength() < 2)
      {
        throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("DropTarget::initialize: Cannot install window event handler")),
                               static_cast<OWeakObject*>(this));
      }

    Any pNSView = aArguments[0];
    sal_uInt64 tmp = 0;
    pNSView >>= tmp;
    mView = (id)tmp;
    mpFrame = [(SalFrameView*)mView getSalFrame];

    mDropTargetHelper = [[DropTargetHelper alloc] initWithDropTarget: this];

    [(id <DraggingDestinationHandler>)mView registerDraggingDestinationHandler:mDropTargetHelper];
    [mView registerForDraggedTypes: mDataFlavorMapper->getAllSupportedPboardTypes()];

    id wnd = [mView window];
    NSWindow* parentWnd = [wnd parentWindow];
    unsigned int topWndStyle = (NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask);
    unsigned int wndStyles = [wnd styleMask] & topWndStyle;

    if (parentWnd == nil && (wndStyles == topWndStyle))
      {
        [wnd registerDraggingDestinationHandler:mDropTargetHelper];
        [wnd registerForDraggedTypes: [NSArray arrayWithObjects: NSFilenamesPboardType, nil]];
      }
  }


  void SAL_CALL DropTarget::addDropTargetListener(const uno::Reference<XDropTargetListener>& dtl)
    throw(RuntimeException)
  {
    rBHelper.addListener(::getCppuType(&dtl), dtl);
  }


  void SAL_CALL DropTarget::removeDropTargetListener(const uno::Reference<XDropTargetListener>& dtl)
    throw(RuntimeException)
  {
    rBHelper.removeListener(::getCppuType(&dtl), dtl);
  }


  sal_Bool SAL_CALL DropTarget::isActive(  ) throw(RuntimeException)
  {
    return mbActive;
  }


  void SAL_CALL DropTarget::setActive(sal_Bool active) throw(RuntimeException)
  {
    mbActive = active;
  }


  sal_Int8 SAL_CALL DropTarget::getDefaultActions() throw(RuntimeException)
  {
    return mDefaultActions;
  }


  void SAL_CALL DropTarget::setDefaultActions(sal_Int8 actions) throw(RuntimeException)
  {
    OSL_ENSURE( actions < 8, "No valid default actions");
    mDefaultActions= actions;
  }


  // XDropTargetDragContext

  void SAL_CALL DropTarget::acceptDrag(sal_Int8 dragOperation) throw (RuntimeException)
  {
    mSelectedDropAction = dragOperation;
  }


  void SAL_CALL DropTarget::rejectDrag() throw (RuntimeException)
  {
    mSelectedDropAction = DNDConstants::ACTION_NONE;
  }


  //XDropTargetDropContext

  void SAL_CALL DropTarget::acceptDrop(sal_Int8 dropOperation) throw( RuntimeException)
  {
    mSelectedDropAction = dropOperation;
  }


  void SAL_CALL DropTarget::rejectDrop() throw (RuntimeException)
  {
    mSelectedDropAction = DNDConstants::ACTION_NONE;
  }


  void SAL_CALL DropTarget::dropComplete(sal_Bool success) throw (RuntimeException)
  {
    // Reset the internal transferable used as shortcut in case this is
    // an internal D&D operation
    DragSource::g_XTransferable = uno::Reference<XTransferable>();
    DragSource::g_DropSuccessSet = true;
    DragSource::g_DropSuccess = success;
  }


  void DropTarget::fire_drop( const DropTargetDropEvent& dte)
  {
      OInterfaceContainerHelper* pContainer= rBHelper.getContainer( getCppuType( (uno::Reference<XDropTargetListener>* )0 ) );
    if( pContainer)
      {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
          {
              uno::Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));

            try { listener->drop( dte); }
            catch(RuntimeException&) {}
          }
      }
  }


  void DropTarget::fire_dragEnter(const DropTargetDragEnterEvent& e)
  {
      OInterfaceContainerHelper* pContainer= rBHelper.getContainer( getCppuType( (uno::Reference<XDropTargetListener>* )0 ) );
    if( pContainer)
      {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
          {
              uno::Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));

            try { listener->dragEnter( e); }
            catch (RuntimeException&) {}
          }
      }
  }


  void DropTarget::fire_dragExit(const DropTargetEvent& dte)
  {
      OInterfaceContainerHelper* pContainer= rBHelper.getContainer( getCppuType( (uno::Reference<XDropTargetListener>* )0 ) );

    if( pContainer)
      {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
          {
              uno::Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));

            try { listener->dragExit( dte); }
            catch (RuntimeException&) {}
          }
      }
  }


  void DropTarget::fire_dragOver(const DropTargetDragEvent& dtde)
  {
      OInterfaceContainerHelper* pContainer= rBHelper.getContainer( getCppuType( (uno::Reference<XDropTargetListener>* )0 ) );
    if( pContainer)
      {
        OInterfaceIteratorHelper iter( *pContainer );
        while( iter.hasMoreElements())
          {
              uno::Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));

            try { listener->dragOver( dtde); }
            catch (RuntimeException&) {}
          }
      }
  }


  void DropTarget::fire_dropActionChanged(const DropTargetDragEvent& dtde)
  {
      OInterfaceContainerHelper* pContainer= rBHelper.getContainer( getCppuType( (uno::Reference<XDropTargetListener>* )0 ) );
    if( pContainer)
      {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
          {
              uno::Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));

            try { listener->dropActionChanged( dtde); }
            catch (RuntimeException&) {}
          }
      }
  }


  // XServiceInfo

  OUString SAL_CALL DropTarget::getImplementationName() throw (RuntimeException)
  {
    return dropTarget_getImplementationName();
  }


  sal_Bool SAL_CALL DropTarget::supportsService( const OUString& ServiceName ) throw (RuntimeException)
  {
    return ServiceName == "com.sun.star.datatransfer.dnd.OleDropTarget";
  }


  Sequence< OUString > SAL_CALL DropTarget::getSupportedServiceNames(  ) throw (RuntimeException)
  {
    return dropTarget_getSupportedServiceNames();
  }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
