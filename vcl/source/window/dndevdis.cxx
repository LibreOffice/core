/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <dndevdis.hxx>
#include <dndlcon.hxx>
#include <window.h>
#include <svdata.hxx>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;





DNDEventDispatcher::DNDEventDispatcher( Window * pTopWindow ):
    m_pTopWindow( pTopWindow ),
    m_pCurrentWindow( NULL )
{
}





DNDEventDispatcher::~DNDEventDispatcher()
{
}

Window* DNDEventDispatcher::findTopLevelWindow(Point location)
{
    SolarMutexGuard aSolarGuard;

    
    
    if( Application::GetSettings().GetLayoutRTL() )
        m_pTopWindow->ImplMirrorFramePos( location );
    Window * pChildWindow = m_pTopWindow->ImplFindWindow( location );

    if( NULL == pChildWindow )
        pChildWindow = m_pTopWindow;

    while( pChildWindow->ImplGetClientWindow() )
        pChildWindow = pChildWindow->ImplGetClientWindow();

    if( pChildWindow->ImplIsAntiparallel() )
    {
        const OutputDevice *pChildWinOutDev = pChildWindow->GetOutDev();
        pChildWinOutDev->ReMirror( location );
    }

    return pChildWindow;
}





void SAL_CALL DNDEventDispatcher::drop( const DropTargetDropEvent& dtde )
    throw(RuntimeException)
{
    osl::MutexGuard aImplGuard( m_aMutex );

    Point location( dtde.LocationX, dtde.LocationY );

    Window* pChildWindow = findTopLevelWindow(location);

    
    if( pChildWindow != m_pCurrentWindow )
    {
        
        fireDragExitEvent( m_pCurrentWindow );

        fireDragEnterEvent( pChildWindow, static_cast < XDropTargetDragContext * > (this),
            dtde.DropAction, location, dtde.SourceActions, m_aDataFlavorList );
    }

    sal_Int32 nListeners = 0;

    
    nListeners = fireDropEvent( pChildWindow, dtde.Context, dtde.DropAction,
        location, dtde.SourceActions, dtde.Transferable );

    
    if( nListeners == 0 ) {
        OSL_TRACE( "rejecting drop due to missing listeners." );
        dtde.Context->rejectDrop();
    }

    
    m_pCurrentWindow = NULL;
    m_aDataFlavorList.realloc( 0 );
}





void SAL_CALL DNDEventDispatcher::dragEnter( const DropTargetDragEnterEvent& dtdee )
    throw(RuntimeException)
{
    osl::MutexGuard aImplGuard( m_aMutex );
    Point location( dtdee.LocationX, dtdee.LocationY );

    Window * pChildWindow = findTopLevelWindow(location);

    
    m_pCurrentWindow = pChildWindow;
    m_aDataFlavorList = dtdee.SupportedDataFlavors;

    
    sal_Int32 nListeners = fireDragEnterEvent( pChildWindow, dtdee.Context, dtdee.DropAction, location,
        dtdee.SourceActions, dtdee.SupportedDataFlavors );

    
    if( nListeners == 0 ) {
        OSL_TRACE( "rejecting drag enter due to missing listeners." );
        dtdee.Context->rejectDrag();
    }

}





void SAL_CALL DNDEventDispatcher::dragExit( const DropTargetEvent& /*dte*/ )
    throw(RuntimeException)
{
    osl::MutexGuard aImplGuard( m_aMutex );

    fireDragExitEvent( m_pCurrentWindow );

    
    m_pCurrentWindow = NULL;
    m_aDataFlavorList.realloc( 0 );
}





void SAL_CALL DNDEventDispatcher::dragOver( const DropTargetDragEvent& dtde )
    throw(RuntimeException)
{
    osl::MutexGuard aImplGuard( m_aMutex );

    Point location( dtde.LocationX, dtde.LocationY );
    sal_Int32 nListeners;

    Window * pChildWindow = findTopLevelWindow(location);

    if( pChildWindow != m_pCurrentWindow )
    {
        
        fireDragExitEvent( m_pCurrentWindow );

        
        m_pCurrentWindow = pChildWindow;

        
        nListeners = fireDragEnterEvent( pChildWindow, dtde.Context, dtde.DropAction, location,
            dtde.SourceActions, m_aDataFlavorList );
    }
    else
    {
        
        nListeners = fireDragOverEvent( pChildWindow, dtde.Context, dtde.DropAction, location,
            dtde.SourceActions );
    }

    
    if( nListeners == 0 )
    {
        OSL_TRACE( "rejecting drag over due to missing listeners." );
        dtde.Context->rejectDrag();
    }
}





void SAL_CALL DNDEventDispatcher::dropActionChanged( const DropTargetDragEvent& dtde )
    throw(RuntimeException)
{
    osl::MutexGuard aImplGuard( m_aMutex );

    Point location( dtde.LocationX, dtde.LocationY );
    sal_Int32 nListeners;

    Window* pChildWindow = findTopLevelWindow(location);

    if( pChildWindow != m_pCurrentWindow )
    {
        
        fireDragExitEvent( m_pCurrentWindow );

        
        m_pCurrentWindow = pChildWindow;

        
        nListeners = fireDragEnterEvent( pChildWindow, dtde.Context, dtde.DropAction, location,
            dtde.SourceActions, m_aDataFlavorList );
    }
    else
    {
        
        nListeners = fireDropActionChangedEvent( pChildWindow, dtde.Context, dtde.DropAction, location,
            dtde.SourceActions );
    }

    
    if( nListeners == 0 )
    {
        OSL_TRACE( "rejecting dropActionChanged due to missing listeners." );
        dtde.Context->rejectDrag();
    }
}






void SAL_CALL DNDEventDispatcher::dragGestureRecognized( const DragGestureEvent& dge )
    throw(RuntimeException)
{
    osl::MutexGuard aImplGuard( m_aMutex );

    Point origin( dge.DragOriginX, dge.DragOriginY );

    Window* pChildWindow = findTopLevelWindow(origin);

    fireDragGestureEvent( pChildWindow, dge.DragSource, dge.Event, origin, dge.DragAction );
}





void SAL_CALL DNDEventDispatcher::disposing( const EventObject& )
    throw(RuntimeException)
{
}





void SAL_CALL DNDEventDispatcher::acceptDrag( sal_Int8 /*dropAction*/ ) throw(RuntimeException)
{
}





void SAL_CALL DNDEventDispatcher::rejectDrag() throw(RuntimeException)
{
}





sal_Int32 DNDEventDispatcher::fireDragEnterEvent( Window *pWindow,
    const Reference< XDropTargetDragContext >& xContext, const sal_Int8 nDropAction,
    const Point& rLocation, const sal_Int8 nSourceActions, const Sequence< DataFlavor >& aFlavorList
)
    throw(RuntimeException)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        SolarMutexClearableGuard aSolarGuard;

        
        pWindow->IncrementLockCount();

        
        Reference< XDropTarget > xDropTarget = pWindow->GetDropTarget();

        if( xDropTarget.is() )
        {
            
            Point relLoc = pWindow->ImplFrameToOutput( rLocation );
            aSolarGuard.clear();

            n = static_cast < DNDListenerContainer * > ( xDropTarget.get() )->fireDragEnterEvent(
                xContext, nDropAction, relLoc.X(), relLoc.Y(), nSourceActions, aFlavorList );
        }
    }

    return n;
}





sal_Int32 DNDEventDispatcher::fireDragOverEvent( Window *pWindow,
    const Reference< XDropTargetDragContext >& xContext, const sal_Int8 nDropAction,
    const Point& rLocation, const sal_Int8 nSourceActions
)
    throw(RuntimeException)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        SolarMutexClearableGuard aSolarGuard;

        
        Reference< XDropTarget > xDropTarget = pWindow->GetDropTarget();

        if( xDropTarget.is() )
        {
            
            Point relLoc = pWindow->ImplFrameToOutput( rLocation );
            aSolarGuard.clear();

            n = static_cast < DNDListenerContainer * > ( xDropTarget.get() )->fireDragOverEvent(
                xContext, nDropAction, relLoc.X(), relLoc.Y(), nSourceActions );
        }
    }

    return n;
}





sal_Int32 DNDEventDispatcher::fireDragExitEvent( Window *pWindow ) throw(RuntimeException)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        SolarMutexClearableGuard aGuard;

        
        Reference< XDropTarget > xDropTarget = pWindow->GetDropTarget();

        aGuard.clear();

        if( xDropTarget.is() )
            n = static_cast < DNDListenerContainer * > ( xDropTarget.get() )->fireDragExitEvent();

        
        pWindow->DecrementLockCount();
    }

    return n;
}





sal_Int32 DNDEventDispatcher::fireDropActionChangedEvent( Window *pWindow,
    const Reference< XDropTargetDragContext >& xContext, const sal_Int8 nDropAction,
    const Point& rLocation, const sal_Int8 nSourceActions
)
    throw(RuntimeException)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        SolarMutexClearableGuard aGuard;

        
        Reference< XDropTarget > xDropTarget = pWindow->GetDropTarget();

        if( xDropTarget.is() )
        {
            
            Point relLoc = pWindow->ImplFrameToOutput( rLocation );
            aGuard.clear();

            n = static_cast < DNDListenerContainer * > ( xDropTarget.get() )->fireDropActionChangedEvent(
                xContext, nDropAction, relLoc.X(), relLoc.Y(), nSourceActions );
        }
    }

    return n;
}





sal_Int32 DNDEventDispatcher::fireDropEvent( Window *pWindow,
    const Reference< XDropTargetDropContext >& xContext, const sal_Int8 nDropAction, const Point& rLocation,
    const sal_Int8 nSourceActions, const Reference< XTransferable >& xTransferable
)
    throw(RuntimeException)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        SolarMutexClearableGuard aGuard;

        
        Reference< XDropTarget > xDropTarget = pWindow->GetDropTarget();

        
        ImplDelData         aDelData;
        pWindow->ImplAddDel( &aDelData );

        if( xDropTarget.is() )
        {
            
            Point relLoc = pWindow->ImplFrameToOutput( rLocation );
            aGuard.clear();

            n = static_cast < DNDListenerContainer * > ( xDropTarget.get() )->fireDropEvent(
                xContext, nDropAction, relLoc.X(), relLoc.Y(), nSourceActions, xTransferable );
        }

        if ( !aDelData.IsDead() )
        {
            pWindow->ImplRemoveDel( &aDelData );
            
            pWindow->DecrementLockCount();
        }

    }

    return n;
}





sal_Int32 DNDEventDispatcher::fireDragGestureEvent( Window *pWindow,
    const Reference< XDragSource >& xSource, const Any event,
    const Point& rOrigin, const sal_Int8 nDragAction
)
    throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32 n = 0;

    if( pWindow && pWindow->IsInputEnabled() && ! pWindow->IsInModalMode() )
    {
        SolarMutexClearableGuard aGuard;

        
        Reference< XDragGestureRecognizer > xDragGestureRecognizer = pWindow->GetDragGestureRecognizer();

        if( xDragGestureRecognizer.is() )
        {
            
            Point relLoc = pWindow->ImplFrameToOutput( rOrigin );
            aGuard.clear();

            n = static_cast < DNDListenerContainer * > ( xDragGestureRecognizer.get() )->fireDragGestureEvent(
                nDragAction, relLoc.X(), relLoc.Y(), xSource, event );
        }

        
        pWindow->DecrementLockCount();
    }

    return n;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
