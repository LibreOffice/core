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

#include <dndeventdispatcher.hxx>
#include <sal/log.hxx>

#include <osl/mutex.hxx>
#include <vcl/dndlistenercontainer.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;

DNDEventDispatcher::DNDEventDispatcher( vcl::Window * pTopWindow ):
    m_pTopWindow( pTopWindow ),
    m_pCurrentWindow( nullptr )
{
}

DNDEventDispatcher::~DNDEventDispatcher()
{
    designate_currentwindow(nullptr);
}

vcl::Window* DNDEventDispatcher::findTopLevelWindow(Point& location)
{
    SolarMutexGuard aSolarGuard;

    // find the window that is toplevel for this coordinates
    // because those coordinates come from outside, they must be mirrored if RTL layout is active
    if( AllSettings::GetLayoutRTL() )
        m_pTopWindow->ImplMirrorFramePos( location );
    vcl::Window * pChildWindow = m_pTopWindow->ImplFindWindow( location );

    if( nullptr == pChildWindow )
        pChildWindow = m_pTopWindow;

    while( pChildWindow->ImplGetClientWindow() )
        pChildWindow = pChildWindow->ImplGetClientWindow();

    if( pChildWindow->GetOutDev()->ImplIsAntiparallel() )
    {
        const OutputDevice *pChildWinOutDev = pChildWindow->GetOutDev();
        pChildWinOutDev->ReMirror( location );
    }

    return pChildWindow;
}

IMPL_LINK(DNDEventDispatcher, WindowEventListener, VclWindowEvent&, rEvent, void)
{
    if (rEvent.GetId() == VclEventId::ObjectDying)
    {
        designate_currentwindow(nullptr);
    }
}

void DNDEventDispatcher::designate_currentwindow(vcl::Window *pWindow)
{
    if (m_pCurrentWindow)
        m_pCurrentWindow->RemoveEventListener(LINK(this, DNDEventDispatcher, WindowEventListener));
    m_pCurrentWindow = pWindow;
    if (m_pCurrentWindow)
        m_pCurrentWindow->AddEventListener(LINK(this, DNDEventDispatcher, WindowEventListener));
}

void SAL_CALL DNDEventDispatcher::drop( const DropTargetDropEvent& dtde )
{
    std::scoped_lock aImplGuard( m_aMutex );

    Point location( dtde.LocationX, dtde.LocationY );

    vcl::Window* pChildWindow = findTopLevelWindow(location);

    // handle the case that drop is in another vcl window than the last dragOver
    if( pChildWindow != m_pCurrentWindow.get() )
    {
        // fire dragExit on listeners of previous window
        fireDragExitEvent( m_pCurrentWindow );

        fireDragEnterEvent( pChildWindow, static_cast < XDropTargetDragContext * > (this),
            dtde.DropAction, location, dtde.SourceActions, m_aDataFlavorList );
    }

    // send drop event to the child window
    sal_Int32 nListeners = fireDropEvent( pChildWindow, dtde.Context, dtde.DropAction,
        location, dtde.SourceActions, dtde.Transferable );

    // reject drop if no listeners found
    if( nListeners == 0 ) {
        SAL_WARN( "vcl", "rejecting drop due to missing listeners." );
        dtde.Context->rejectDrop();
    }

    // this is a drop -> no further drag overs
    designate_currentwindow(nullptr);
    m_aDataFlavorList.realloc( 0 );
}

void SAL_CALL DNDEventDispatcher::dragEnter( const DropTargetDragEnterEvent& dtdee )
{
    std::scoped_lock aImplGuard( m_aMutex );
    Point location( dtdee.LocationX, dtdee.LocationY );

    vcl::Window * pChildWindow = findTopLevelWindow(location);

    // assume pointer write operation to be atomic
    designate_currentwindow(pChildWindow);
    m_aDataFlavorList = dtdee.SupportedDataFlavors;

    // fire dragEnter on listeners of current window
    sal_Int32 nListeners = fireDragEnterEvent( pChildWindow, dtdee.Context, dtdee.DropAction, location,
        dtdee.SourceActions, dtdee.SupportedDataFlavors );

    // reject drag if no listener found
    if( nListeners == 0 ) {
        SAL_WARN( "vcl", "rejecting drag enter due to missing listeners." );
        dtdee.Context->rejectDrag();
    }

}

void SAL_CALL DNDEventDispatcher::dragExit( const DropTargetEvent& /*dte*/ )
{
    std::scoped_lock aImplGuard( m_aMutex );

    fireDragExitEvent( m_pCurrentWindow );

    // reset member values
    designate_currentwindow(nullptr);
    m_aDataFlavorList.realloc( 0 );
}

void SAL_CALL DNDEventDispatcher::dragOver( const DropTargetDragEvent& dtde )
{
    std::scoped_lock aImplGuard( m_aMutex );

    Point location( dtde.LocationX, dtde.LocationY );
    sal_Int32 nListeners;

    vcl::Window * pChildWindow = findTopLevelWindow(location);

    if( pChildWindow != m_pCurrentWindow.get() )
    {
        // fire dragExit on listeners of previous window
        fireDragExitEvent( m_pCurrentWindow );

        // remember new window
        designate_currentwindow(pChildWindow);

        // fire dragEnter on listeners of current window
        nListeners = fireDragEnterEvent( pChildWindow, dtde.Context, dtde.DropAction, location,
            dtde.SourceActions, m_aDataFlavorList );
    }
    else
    {
        // fire dragOver on listeners of current window
        nListeners = fireDragOverEvent( pChildWindow, dtde.Context, dtde.DropAction, location,
            dtde.SourceActions );
    }

    // reject drag if no listener found
    if( nListeners == 0 )
    {
        SAL_WARN( "vcl", "rejecting drag over due to missing listeners." );
        dtde.Context->rejectDrag();
    }
}

void SAL_CALL DNDEventDispatcher::dropActionChanged( const DropTargetDragEvent& dtde )
{
    std::scoped_lock aImplGuard( m_aMutex );

    Point location( dtde.LocationX, dtde.LocationY );
    sal_Int32 nListeners;

    vcl::Window* pChildWindow = findTopLevelWindow(location);

    if( pChildWindow != m_pCurrentWindow.get() )
    {
        // fire dragExit on listeners of previous window
        fireDragExitEvent( m_pCurrentWindow );

        // remember new window
        designate_currentwindow(pChildWindow);

        // fire dragEnter on listeners of current window
        nListeners = fireDragEnterEvent( pChildWindow, dtde.Context, dtde.DropAction, location,
            dtde.SourceActions, m_aDataFlavorList );
    }
    else
    {
        // fire dropActionChanged on listeners of current window
        nListeners = fireDropActionChangedEvent( pChildWindow, dtde.Context, dtde.DropAction, location,
            dtde.SourceActions );
    }

    // reject drag if no listener found
    if( nListeners == 0 )
    {
        SAL_WARN( "vcl", "rejecting dropActionChanged due to missing listeners." );
        dtde.Context->rejectDrag();
    }
}

void SAL_CALL DNDEventDispatcher::dragGestureRecognized( const DragGestureEvent& dge )
{
    std::scoped_lock aImplGuard( m_aMutex );

    Point origin( dge.DragOriginX, dge.DragOriginY );

    vcl::Window* pChildWindow = findTopLevelWindow(origin);

    fireDragGestureEvent( pChildWindow, dge.DragSource, dge.Event, origin, dge.DragAction );
}

void SAL_CALL DNDEventDispatcher::disposing( const EventObject& )
{
}

void SAL_CALL DNDEventDispatcher::acceptDrag( sal_Int8 /*dropAction*/ )
{
}

void SAL_CALL DNDEventDispatcher::rejectDrag()
{
}

sal_Int32 DNDEventDispatcher::fireDragEnterEvent( vcl::Window *pWindow,
    const Reference< XDropTargetDragContext >& xContext, const sal_Int8 nDropAction,
    const Point& rLocation, const sal_Int8 nSourceActions, const Sequence< DataFlavor >& aFlavorList
)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        SolarMutexClearableGuard aSolarGuard;

        // query DropTarget from window
        rtl::Reference<DNDListenerContainer> pDropTarget = pWindow->GetDropTarget();

        if (pDropTarget.is())
        {
            // retrieve relative mouse position
            Point relLoc = pWindow->ScreenToOutputPixel( rLocation );
            aSolarGuard.clear();

            n = pDropTarget->fireDragEnterEvent(xContext, nDropAction, relLoc.X(), relLoc.Y(),
                                                nSourceActions, aFlavorList);
        }
    }

    return n;
}

sal_Int32 DNDEventDispatcher::fireDragOverEvent( vcl::Window *pWindow,
    const Reference< XDropTargetDragContext >& xContext, const sal_Int8 nDropAction,
    const Point& rLocation, const sal_Int8 nSourceActions
)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        SolarMutexClearableGuard aSolarGuard;

        // query DropTarget from window
        rtl::Reference<DNDListenerContainer> pDropTarget = pWindow->GetDropTarget();

        if (pDropTarget.is())
        {
            // retrieve relative mouse position
            Point relLoc = pWindow->ScreenToOutputPixel( rLocation );
            aSolarGuard.clear();

            n = pDropTarget->fireDragOverEvent(xContext, nDropAction, relLoc.X(), relLoc.Y(),
                                               nSourceActions);
        }
    }

    return n;
}

sal_Int32 DNDEventDispatcher::fireDragExitEvent( vcl::Window *pWindow )
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        SolarMutexClearableGuard aGuard;

        // query DropTarget from window
        rtl::Reference<DNDListenerContainer> pDropTarget = pWindow->GetDropTarget();

        aGuard.clear();

        if (pDropTarget.is())
            n = pDropTarget->fireDragExitEvent();
    }

    return n;
}

sal_Int32 DNDEventDispatcher::fireDropActionChangedEvent( vcl::Window *pWindow,
    const Reference< XDropTargetDragContext >& xContext, const sal_Int8 nDropAction,
    const Point& rLocation, const sal_Int8 nSourceActions
)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        SolarMutexClearableGuard aGuard;

        // query DropTarget from window
        rtl::Reference<DNDListenerContainer> pDropTarget = pWindow->GetDropTarget();

        if (pDropTarget.is())
        {
            // retrieve relative mouse position
            Point relLoc = pWindow->ScreenToOutputPixel( rLocation );
            aGuard.clear();

            n = pDropTarget->fireDropActionChangedEvent(xContext, nDropAction, relLoc.X(),
                                                        relLoc.Y(), nSourceActions);
        }
    }

    return n;
}

sal_Int32 DNDEventDispatcher::fireDropEvent( vcl::Window *pWindow,
    const Reference< XDropTargetDropContext >& xContext, const sal_Int8 nDropAction, const Point& rLocation,
    const sal_Int8 nSourceActions, const Reference< XTransferable >& xTransferable
)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        SolarMutexClearableGuard aGuard;

        // query DropTarget from window
        rtl::Reference<DNDListenerContainer> pDropTarget = pWindow->GetDropTarget();

        // window may be destroyed in drop event handler
        VclPtr<vcl::Window> xPreventDelete = pWindow;

        if (pDropTarget.is())
        {
            // retrieve relative mouse position
            Point relLoc = pWindow->ScreenToOutputPixel( rLocation );
            aGuard.clear();

            n = pDropTarget->fireDropEvent(xContext, nDropAction, relLoc.X(), relLoc.Y(),
                                           nSourceActions, xTransferable);
        }
    }

    return n;
}

sal_Int32 DNDEventDispatcher::fireDragGestureEvent( vcl::Window *pWindow,
    const Reference< XDragSource >& xSource, const Any& event,
    const Point& rOrigin, const sal_Int8 nDragAction
)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        SolarMutexClearableGuard aGuard;

        // query DropTarget from window
        rtl::Reference<DNDListenerContainer> pDropTarget = pWindow->GetDropTarget();

        if (pDropTarget.is())
        {
            // retrieve relative mouse position
            Point relLoc = pWindow->ScreenToOutputPixel( rOrigin );
            aGuard.clear();

            n = pDropTarget->fireDragGestureEvent(nDragAction, relLoc.X(), relLoc.Y(), xSource,
                                                  event);
        }
    }

    return n;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
