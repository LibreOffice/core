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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <dndevdis.hxx>
#include <dndlcon.hxx>
#include <window.h>
#include <svdata.hxx>

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::osl;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;

//==================================================================================================
// DNDEventDispatcher::DNDEventDispatcher
//==================================================================================================

DNDEventDispatcher::DNDEventDispatcher( Window * pTopWindow ):
    m_pTopWindow( pTopWindow ),
    m_pCurrentWindow( NULL )
{
}

//==================================================================================================
// DNDEventDispatcher::~DNDEventDispatcher
//==================================================================================================

DNDEventDispatcher::~DNDEventDispatcher()
{
}

//==================================================================================================
// DNDEventDispatcher::drop
//==================================================================================================

void SAL_CALL DNDEventDispatcher::drop( const DropTargetDropEvent& dtde )
    throw(RuntimeException)
{
    MutexGuard aImplGuard( m_aMutex );

    Point location( dtde.LocationX, dtde.LocationY );

    // find the window that is toplevel for this coordinates
    OClearableGuard aSolarGuard( Application::GetSolarMutex() );

    // because those coordinates come from outside, they must be mirrored if RTL layout is active
    if( Application::GetSettings().GetLayoutRTL() )
        m_pTopWindow->ImplMirrorFramePos( location );
    Window * pChildWindow = m_pTopWindow->ImplFindWindow( location );

    if( NULL == pChildWindow )
        pChildWindow = m_pTopWindow;

    while( pChildWindow->ImplGetClientWindow() )
        pChildWindow = pChildWindow->ImplGetClientWindow();

    if( pChildWindow->ImplIsAntiparallel() )
        pChildWindow->ImplReMirror( location );

    aSolarGuard.clear();

    // handle the case that drop is in an other vcl window than the last dragOver
    if( pChildWindow != m_pCurrentWindow )
    {
        // fire dragExit on listeners of previous window
        fireDragExitEvent( m_pCurrentWindow );

        fireDragEnterEvent( pChildWindow, static_cast < XDropTargetDragContext * > (this),
            dtde.DropAction, location, dtde.SourceActions, m_aDataFlavorList );
    }

    sal_Int32 nListeners = 0;

    // send drop event to the child window
    nListeners = fireDropEvent( pChildWindow, dtde.Context, dtde.DropAction,
        location, dtde.SourceActions, dtde.Transferable );

    // reject drop if no listeners found
    if( nListeners == 0 ) {
        OSL_TRACE( "rejecting drop due to missing listeners." );
        dtde.Context->rejectDrop();
    }

    // this is a drop -> no further drag overs
    m_pCurrentWindow = NULL;
    m_aDataFlavorList.realloc( 0 );
}

//==================================================================================================
// DNDEventDispatcher::dragEnter
//==================================================================================================

void SAL_CALL DNDEventDispatcher::dragEnter( const DropTargetDragEnterEvent& dtdee )
    throw(RuntimeException)
{
    MutexGuard aImplGuard( m_aMutex );
    Point location( dtdee.LocationX, dtdee.LocationY );

    // find the window that is toplevel for this coordinates
    OClearableGuard aSolarGuard( Application::GetSolarMutex() );

    // because those coordinates come from outside, they must be mirrored if RTL layout is active
    if( Application::GetSettings().GetLayoutRTL() )
        m_pTopWindow->ImplMirrorFramePos( location );
    Window * pChildWindow = m_pTopWindow->ImplFindWindow( location );

    if( NULL == pChildWindow )
        pChildWindow = m_pTopWindow;

    while( pChildWindow->ImplGetClientWindow() )
        pChildWindow = pChildWindow->ImplGetClientWindow();

    if( pChildWindow->ImplIsAntiparallel() )
        pChildWindow->ImplReMirror( location );

    aSolarGuard.clear();

    // assume pointer write operation to be atomic
    m_pCurrentWindow = pChildWindow;
    m_aDataFlavorList = dtdee.SupportedDataFlavors;

    // fire dragEnter on listeners of current window
    sal_Int32 nListeners = fireDragEnterEvent( pChildWindow, dtdee.Context, dtdee.DropAction, location,
        dtdee.SourceActions, dtdee.SupportedDataFlavors );

    // reject drag if no listener found
    if( nListeners == 0 ) {
        OSL_TRACE( "rejecting drag enter due to missing listeners." );
        dtdee.Context->rejectDrag();
    }

}

//==================================================================================================
// DNDEventDispatcher::dragExit
//==================================================================================================

void SAL_CALL DNDEventDispatcher::dragExit( const DropTargetEvent& /*dte*/ )
    throw(RuntimeException)
{
    MutexGuard aImplGuard( m_aMutex );

    fireDragExitEvent( m_pCurrentWindow );

    // reset member values
    m_pCurrentWindow = NULL;
    m_aDataFlavorList.realloc( 0 );
}

//==================================================================================================
// DNDEventDispatcher::dragOver
//==================================================================================================

void SAL_CALL DNDEventDispatcher::dragOver( const DropTargetDragEvent& dtde )
    throw(RuntimeException)
{
    MutexGuard aImplGuard( m_aMutex );

    Point location( dtde.LocationX, dtde.LocationY );
    sal_Int32 nListeners;

    // find the window that is toplevel for this coordinates
    OClearableGuard aSolarGuard( Application::GetSolarMutex() );

    // because those coordinates come from outside, they must be mirrored if RTL layout is active
    if( Application::GetSettings().GetLayoutRTL() )
        m_pTopWindow->ImplMirrorFramePos( location );
    Window * pChildWindow = m_pTopWindow->ImplFindWindow( location );

    if( NULL == pChildWindow )
        pChildWindow = m_pTopWindow;

    while( pChildWindow->ImplGetClientWindow() )
        pChildWindow = pChildWindow->ImplGetClientWindow();

    if( pChildWindow->ImplIsAntiparallel() )
        pChildWindow->ImplReMirror( location );

    aSolarGuard.clear();

    if( pChildWindow != m_pCurrentWindow )
    {
        // fire dragExit on listeners of previous window
        fireDragExitEvent( m_pCurrentWindow );

        // remember new window
        m_pCurrentWindow = pChildWindow;

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
        OSL_TRACE( "rejecting drag over due to missing listeners." );
        dtde.Context->rejectDrag();
    }
}

//==================================================================================================
// DNDEventDispatcher::dropActionChanged
//==================================================================================================

void SAL_CALL DNDEventDispatcher::dropActionChanged( const DropTargetDragEvent& dtde )
    throw(RuntimeException)
{
    MutexGuard aImplGuard( m_aMutex );

    Point location( dtde.LocationX, dtde.LocationY );
    sal_Int32 nListeners;

    // find the window that is toplevel for this coordinates
    OClearableGuard aSolarGuard( Application::GetSolarMutex() );

    // because those coordinates come from outside, they must be mirrored if RTL layout is active
    if( Application::GetSettings().GetLayoutRTL() )
        m_pTopWindow->ImplMirrorFramePos( location );
    Window * pChildWindow = m_pTopWindow->ImplFindWindow( location );

    if( NULL == pChildWindow )
        pChildWindow = m_pTopWindow;

    while( pChildWindow->ImplGetClientWindow() )
        pChildWindow = pChildWindow->ImplGetClientWindow();

    if( pChildWindow->ImplIsAntiparallel() )
        pChildWindow->ImplReMirror( location );

    aSolarGuard.clear();

    if( pChildWindow != m_pCurrentWindow )
    {
        // fire dragExit on listeners of previous window
        fireDragExitEvent( m_pCurrentWindow );

        // remember new window
        m_pCurrentWindow = pChildWindow;

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
        OSL_TRACE( "rejecting dropActionChanged due to missing listeners." );
        dtde.Context->rejectDrag();
    }
}


//==================================================================================================
// DNDEventDispatcher::dragGestureRecognized
//==================================================================================================

void SAL_CALL DNDEventDispatcher::dragGestureRecognized( const DragGestureEvent& dge )
    throw(RuntimeException)
{   MutexGuard aImplGuard( m_aMutex );

    Point origin( dge.DragOriginX, dge.DragOriginY );

    // find the window that is toplevel for this coordinates
    OClearableGuard aSolarGuard( Application::GetSolarMutex() );

    // because those coordinates come from outside, they must be mirrored if RTL layout is active
    if( Application::GetSettings().GetLayoutRTL() )
        m_pTopWindow->ImplMirrorFramePos( origin );
    Window * pChildWindow = m_pTopWindow->ImplFindWindow( origin );

    if( NULL == pChildWindow )
        pChildWindow = m_pTopWindow;

    while( pChildWindow->ImplGetClientWindow() )
        pChildWindow = pChildWindow->ImplGetClientWindow();

    if( pChildWindow->ImplIsAntiparallel() )
        pChildWindow->ImplReMirror( origin );

    aSolarGuard.clear();

    fireDragGestureEvent( pChildWindow, dge.DragSource, dge.Event, origin, dge.DragAction );
}

//==================================================================================================
// DNDEventDispatcher::disposing
//==================================================================================================

void SAL_CALL DNDEventDispatcher::disposing( const EventObject& )
    throw(RuntimeException)
{
}

//==================================================================================================
// DNDEventDispatcher::acceptDrag
//==================================================================================================

void SAL_CALL DNDEventDispatcher::acceptDrag( sal_Int8 /*dropAction*/ ) throw(RuntimeException)
{
}

//==================================================================================================
// DNDEventDispatcher::rejectDrag
//==================================================================================================

void SAL_CALL DNDEventDispatcher::rejectDrag() throw(RuntimeException)
{
}

//==================================================================================================
// DNDEventDispatcher::fireDragEnterEvent
//==================================================================================================

sal_Int32 DNDEventDispatcher::fireDragEnterEvent( Window *pWindow,
    const Reference< XDropTargetDragContext >& xContext, const sal_Int8 nDropAction,
    const Point& rLocation, const sal_Int8 nSourceActions, const Sequence< DataFlavor >& aFlavorList
)
    throw(RuntimeException)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        OClearableGuard aGuard( Application::GetSolarMutex() );

        // set an UI lock
        pWindow->IncrementLockCount();

        // query DropTarget from window
        Reference< XDropTarget > xDropTarget = pWindow->GetDropTarget();

        if( xDropTarget.is() )
        {
            // retrieve relative mouse position
            Point relLoc = pWindow->ImplFrameToOutput( rLocation );
            aGuard.clear();

            n = static_cast < DNDListenerContainer * > ( xDropTarget.get() )->fireDragEnterEvent(
                xContext, nDropAction, relLoc.X(), relLoc.Y(), nSourceActions, aFlavorList );
        }
    }

    return n;
}

//==================================================================================================
// DNDEventDispatcher::fireDragOverEvent
//==================================================================================================

sal_Int32 DNDEventDispatcher::fireDragOverEvent( Window *pWindow,
    const Reference< XDropTargetDragContext >& xContext, const sal_Int8 nDropAction,
    const Point& rLocation, const sal_Int8 nSourceActions
)
    throw(RuntimeException)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        OClearableGuard aGuard( Application::GetSolarMutex() );

        // query DropTarget from window
        Reference< XDropTarget > xDropTarget = pWindow->GetDropTarget();

        if( xDropTarget.is() )
        {
            // retrieve relative mouse position
            Point relLoc = pWindow->ImplFrameToOutput( rLocation );
            aGuard.clear();

            n = static_cast < DNDListenerContainer * > ( xDropTarget.get() )->fireDragOverEvent(
                xContext, nDropAction, relLoc.X(), relLoc.Y(), nSourceActions );
        }
    }

    return n;
}

//==================================================================================================
// DNDEventDispatcher::fireDragExitEvent
//==================================================================================================

sal_Int32 DNDEventDispatcher::fireDragExitEvent( Window *pWindow ) throw(RuntimeException)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        OClearableGuard aGuard( Application::GetSolarMutex() );

        // query DropTarget from window
        Reference< XDropTarget > xDropTarget = pWindow->GetDropTarget();

        aGuard.clear();

        if( xDropTarget.is() )
            n = static_cast < DNDListenerContainer * > ( xDropTarget.get() )->fireDragExitEvent();

        // release UI lock
        pWindow->DecrementLockCount();
    }

    return n;
}

//==================================================================================================
// DNDEventDispatcher::fireDropActionChangedEvent
//==================================================================================================

sal_Int32 DNDEventDispatcher::fireDropActionChangedEvent( Window *pWindow,
    const Reference< XDropTargetDragContext >& xContext, const sal_Int8 nDropAction,
    const Point& rLocation, const sal_Int8 nSourceActions
)
    throw(RuntimeException)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        OClearableGuard aGuard( Application::GetSolarMutex() );

        // query DropTarget from window
        Reference< XDropTarget > xDropTarget = pWindow->GetDropTarget();

        if( xDropTarget.is() )
        {
            // retrieve relative mouse position
            Point relLoc = pWindow->ImplFrameToOutput( rLocation );
            aGuard.clear();

            n = static_cast < DNDListenerContainer * > ( xDropTarget.get() )->fireDropActionChangedEvent(
                xContext, nDropAction, relLoc.X(), relLoc.Y(), nSourceActions );
        }
    }

    return n;
}

//==================================================================================================
// DNDEventDispatcher::fireDropEvent
//==================================================================================================

sal_Int32 DNDEventDispatcher::fireDropEvent( Window *pWindow,
    const Reference< XDropTargetDropContext >& xContext, const sal_Int8 nDropAction, const Point& rLocation,
    const sal_Int8 nSourceActions, const Reference< XTransferable >& xTransferable
)
    throw(RuntimeException)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        OClearableGuard aGuard( Application::GetSolarMutex() );

        // query DropTarget from window
        Reference< XDropTarget > xDropTarget = pWindow->GetDropTarget();

        // window may be destroyed in drop event handler
        ImplDelData         aDelData;
        pWindow->ImplAddDel( &aDelData );

        if( xDropTarget.is() )
        {
            // retrieve relative mouse position
            Point relLoc = pWindow->ImplFrameToOutput( rLocation );
            aGuard.clear();

            n = static_cast < DNDListenerContainer * > ( xDropTarget.get() )->fireDropEvent(
                xContext, nDropAction, relLoc.X(), relLoc.Y(), nSourceActions, xTransferable );
        }

        if ( !aDelData.IsDelete() )
        {
            pWindow->ImplRemoveDel( &aDelData );
            // release UI lock
            pWindow->DecrementLockCount();
        }

    }

    return n;
}

//==================================================================================================
// DNDEventDispatcher::fireDragGestureRecognized
//==================================================================================================

sal_Int32 DNDEventDispatcher::fireDragGestureEvent( Window *pWindow,
    const Reference< XDragSource >& xSource, const Any event,
    const Point& rOrigin, const sal_Int8 nDragAction
)
    throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        OClearableGuard aGuard( Application::GetSolarMutex() );

        // query DropTarget from window
        Reference< XDragGestureRecognizer > xDragGestureRecognizer = pWindow->GetDragGestureRecognizer();

        if( xDragGestureRecognizer.is() )
        {
            // retrieve relative mouse position
            Point relLoc = pWindow->ImplFrameToOutput( rOrigin );
            aGuard.clear();

            n = static_cast < DNDListenerContainer * > ( xDragGestureRecognizer.get() )->fireDragGestureEvent(
                nDragAction, relLoc.X(), relLoc.Y(), xSource, event );
        }

        // release UI lock
        pWindow->DecrementLockCount();
    }

    return n;
}
