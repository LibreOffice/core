/*************************************************************************
 *
 *  $RCSfile: dndevdis.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obr $ $Date: 2001-02-20 11:17:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <dndevdis.hxx>
#include <dndlcon.hxx>

#include <vos/mutex.hxx>
#include <svapp.hxx>

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
    Window * pChildWindow = m_pTopWindow->ImplFindWindow( location );
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
    Window * pChildWindow = m_pTopWindow->ImplFindWindow( location );
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

void SAL_CALL DNDEventDispatcher::dragExit( const DropTargetEvent& dte )
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
    Window * pChildWindow = m_pTopWindow->ImplFindWindow( location );
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
    Window * pChildWindow = m_pTopWindow->ImplFindWindow( location );
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
// DNDEventDispatcher::disposing
//==================================================================================================

void SAL_CALL DNDEventDispatcher::disposing( const EventObject& eo )
    throw(RuntimeException)
{
}

//==================================================================================================
// DNDEventDispatcher::acceptDrag
//==================================================================================================

void SAL_CALL DNDEventDispatcher::acceptDrag( sal_Int8 dropAction ) throw(RuntimeException)
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

    if( pWindow )
    {
        OClearableGuard aGuard( Application::GetSolarMutex() );

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

    if( pWindow )
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

    if( pWindow )
    {
        OClearableGuard aGuard( Application::GetSolarMutex() );

        // query DropTarget from window
        Reference< XDropTarget > xDropTarget = pWindow->GetDropTarget();

        aGuard.clear();

        if( xDropTarget.is() )
            n = static_cast < DNDListenerContainer * > ( xDropTarget.get() )->fireDragExitEvent();
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

    if( pWindow )
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

    if( pWindow )
    {
        OClearableGuard aGuard( Application::GetSolarMutex() );

        // query DropTarget from window
        Reference< XDropTarget > xDropTarget = pWindow->GetDropTarget();

        if( xDropTarget.is() )
        {
            // retrieve relative mouse position
            Point relLoc = pWindow->ImplFrameToOutput( rLocation );
            aGuard.clear();

            n = static_cast < DNDListenerContainer * > ( xDropTarget.get() )->fireDropEvent(
                xContext, nDropAction, relLoc.X(), relLoc.Y(), nSourceActions, xTransferable );
        }
    }

    return n;
}
