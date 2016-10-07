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
#include <com/sun/star/datatransfer/dnd/DropTargetDragEnterEvent.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include "clipboard.hxx"
#include "DropTarget.hxx"
#include "DragActionConversion.hxx"
#include "DragSource.hxx"
#include <rtl/ustring.h>
#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>
#include <osx/salframe.h>
#include <osx/salframeview.h>
#include <cppuhelper/supportsservice.hxx>

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

OUString dropTarget_getImplementationName()
{
    return OUString("com.sun.star.comp.datatransfer.dnd.OleDropTarget_V1");
}

Sequence<OUString> dropTarget_getSupportedServiceNames()
{
    return { OUString("com.sun.star.datatransfer.dnd.OleDropTarget") };
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
    (void) sender;
    return DropTarget::prepareForDragOperation();
}

-(BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    (void) sender;
    return mDropTarget->performDragOperation();
}

-(void)concludeDragOperation:(id <NSDraggingInfo>)sender
{
    mDropTarget->concludeDragOperation(sender);
}

@end

DropTarget::DropTarget() :
    WeakComponentImplHelper<XInitialization, XDropTarget, XDropTargetDragContext, XDropTargetDropContext, XServiceInfo>(m_aMutex),
    mView(nil),
    mpFrame(nullptr),
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
        NSPoint mouseLoc = [NSEvent mouseLocation];

        id wnd = [mView window];
#if MACOSX_SDK_VERSION < 1070
        NSPoint dragLocation = [mView convertPoint:[wnd convertScreenToBase:mouseLoc] fromView:nil];
#else
        NSPoint dragLocation = [mView convertPoint:[wnd convertRectFromScreen:NSMakeRect(mouseLoc.x, mouseLoc.y, 1, 1)].origin fromView:nil];
#endif
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
        NSPoint mouseLoc = [NSEvent mouseLocation];

        id wnd = [mView window];
#if MACOSX_SDK_VERSION < 1070
        NSPoint dragLocation = [mView convertPoint:[wnd convertScreenToBase:mouseLoc] fromView:nil];
#else
        NSPoint dragLocation = [mView convertPoint:[wnd convertRectFromScreen:NSMakeRect(mouseLoc.x, mouseLoc.y, 1, 1)].origin fromView:nil];
#endif
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

    if (dragOp == NSDragOperationNone)
#if MACOSX_SDK_VERSION < 1060
        SetThemeCursor(kThemeNotAllowedCursor);
#else
        [[NSCursor operationNotAllowedCursor] set];
#endif
    else if (dragOp == NSDragOperationCopy)
#if MACOSX_SDK_VERSION < 1060
        SetThemeCursor(kThemeCopyArrowCursor);
#else
        [[NSCursor dragCopyCursor] set];
#endif
    else
        [[NSCursor arrowCursor] set];

    return dragOp;
}

void DropTarget::draggingExited(id /*sender*/)
{
    DropTargetEvent dte(static_cast<OWeakObject*>(this), 0);
    fire_dragExit(dte);
    mDragSourceSupportedActions = DNDConstants::ACTION_NONE;
    mSelectedDropAction = DNDConstants::ACTION_NONE;
    [[NSCursor arrowCursor] set];
}

BOOL DropTarget::prepareForDragOperation()
{
    return 1;
}

BOOL DropTarget::performDragOperation()
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
        NSPoint mouseLoc = [NSEvent mouseLocation];

        id wnd = [mView window];
#if MACOSX_SDK_VERSION < 1070
        NSPoint dragLocation = [mView convertPoint:[wnd convertScreenToBase:mouseLoc] fromView:nil];
#else
        NSPoint dragLocation = [mView convertPoint:[wnd convertRectFromScreen:NSMakeRect(mouseLoc.x, mouseLoc.y, 1, 1)].origin fromView:nil];
#endif
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
    mXCurrentDragClipboard.clear();
    [[NSCursor arrowCursor] set];
}

// called from WeakComponentImplHelperX::dispose
// WeakComponentImplHelper calls disposing before it destroys
// itself.
void SAL_CALL DropTarget::disposing()
{
}

void SAL_CALL DropTarget::initialize(const Sequence< Any >& aArguments)
    throw(Exception, std::exception)
{
    if (aArguments.getLength() < 2)
    {
        throw RuntimeException("DropTarget::initialize: Cannot install window event handler",
                               static_cast<OWeakObject*>(this));
    }

    Any pNSView = aArguments[0];
    sal_uInt64 tmp = 0;
    pNSView >>= tmp;
    mView = reinterpret_cast<id>(tmp);
    mpFrame = [(SalFrameView*)mView getSalFrame];

    mDropTargetHelper = [[DropTargetHelper alloc] initWithDropTarget: this];

    [(id <DraggingDestinationHandler>)mView registerDraggingDestinationHandler:mDropTargetHelper];
    [mView registerForDraggedTypes: DataFlavorMapper::getAllSupportedPboardTypes()];

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
    throw(RuntimeException, std::exception)
{
    rBHelper.addListener(cppu::UnoType<decltype(dtl)>::get(), dtl);
}

void SAL_CALL DropTarget::removeDropTargetListener(const uno::Reference<XDropTargetListener>& dtl)
    throw(RuntimeException, std::exception)
{
    rBHelper.removeListener(cppu::UnoType<decltype(dtl)>::get(), dtl);
}

sal_Bool SAL_CALL DropTarget::isActive(  ) throw(RuntimeException, std::exception)
{
    return mbActive;
}

void SAL_CALL DropTarget::setActive(sal_Bool active) throw(RuntimeException, std::exception)
{
    mbActive = active;
}

sal_Int8 SAL_CALL DropTarget::getDefaultActions() throw(RuntimeException, std::exception)
{
    return mDefaultActions;
}

void SAL_CALL DropTarget::setDefaultActions(sal_Int8 actions) throw(RuntimeException, std::exception)
{
    OSL_ENSURE( actions < 8, "No valid default actions");
    mDefaultActions= actions;
}

void SAL_CALL DropTarget::acceptDrag(sal_Int8 dragOperation) throw (RuntimeException, std::exception)
{
    mSelectedDropAction = dragOperation;
}

void SAL_CALL DropTarget::rejectDrag() throw (RuntimeException, std::exception)
{
    mSelectedDropAction = DNDConstants::ACTION_NONE;
}

void SAL_CALL DropTarget::acceptDrop(sal_Int8 dropOperation) throw( RuntimeException, std::exception)
{
    mSelectedDropAction = dropOperation;
}

void SAL_CALL DropTarget::rejectDrop() throw (RuntimeException, std::exception)
{
    mSelectedDropAction = DNDConstants::ACTION_NONE;
}

void SAL_CALL DropTarget::dropComplete(sal_Bool success) throw (RuntimeException, std::exception)
{
    // Reset the internal transferable used as shortcut in case this is
    // an internal D&D operation
    DragSource::g_XTransferable.clear();
    DragSource::g_DropSuccessSet = true;
    DragSource::g_DropSuccess = success;
}

void DropTarget::fire_drop( const DropTargetDropEvent& dte)
{
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( cppu::UnoType<XDropTargetListener>::get());
    if( pContainer)
    {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
        {
            uno::Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));
#ifdef __OBJC__
            @try {
#else
            try {
#endif
                listener->drop( dte);
            }
#ifdef __OBJC__
            @catch(...) {}
#else
            catch(RuntimeException&) {}
#endif
        }
    }
}

void DropTarget::fire_dragEnter(const DropTargetDragEnterEvent& e)
{
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( cppu::UnoType<XDropTargetListener>::get());
    if( pContainer)
    {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
        {
            uno::Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));
#ifdef __OBJC__
            @try {
#else
            try {
#endif
                listener->dragEnter( e);
            }
#ifdef __OBJC__
            @catch(...) {}
#else
            catch (RuntimeException&) {}
#endif
        }
    }
}

void DropTarget::fire_dragExit(const DropTargetEvent& dte)
{
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( cppu::UnoType<XDropTargetListener>::get());

    if( pContainer)
    {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
        {
            uno::Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));
#ifdef __OBJC__
            @try {
#else
            try {
#endif
                listener->dragExit( dte);
            }
#ifdef __OBJC__
            @catch(...) {}
#else
            catch (RuntimeException&) {}
#endif
        }
    }
}

void DropTarget::fire_dragOver(const DropTargetDragEvent& dtde)
{
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( cppu::UnoType<XDropTargetListener>::get());
    if( pContainer)
    {
        OInterfaceIteratorHelper iter( *pContainer );
        while( iter.hasMoreElements())
        {
            uno::Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));
#ifdef __OBJC__
            @try {
#else
            try {
#endif
                listener->dragOver( dtde);
            }
#ifdef __OBJC__
            @catch(...) {}
#else
            catch (RuntimeException&) {}
#endif
        }
    }
}

void DropTarget::fire_dropActionChanged(const DropTargetDragEvent& dtde)
{
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( cppu::UnoType<XDropTargetListener>::get());
    if( pContainer)
    {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
        {
            uno::Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));
#ifdef __OBJC__
            @try {
#else
            try {
#endif
                listener->dropActionChanged( dtde);
            }
#ifdef __OBJC__
            @catch(...) {}
#else
            catch (RuntimeException&) {}
#endif
        }
    }
}

OUString SAL_CALL DropTarget::getImplementationName() throw (RuntimeException, std::exception)
{
    return dropTarget_getImplementationName();
}

sal_Bool SAL_CALL DropTarget::supportsService( const OUString& ServiceName ) throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL DropTarget::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    return dropTarget_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
