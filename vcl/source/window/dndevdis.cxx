/*************************************************************************
 *
 *  $RCSfile: dndevdis.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obr $ $Date: 2001-02-09 15:59:18 $
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
    Point location( dtde.Location.X, dtde.Location.Y );
    sal_Bool bSendDragEnter = sal_False;

    // find the window that is toplevel for this coordinates
    OClearableGuard aGuard( Application::GetSolarMutex() );
    Window * pChildWindow = m_pTopWindow->ImplFindWindow( location );
    aGuard.clear();

    // handle the case that drop is in an other vcl window than the last dragOver
    if( pChildWindow != m_pCurrentWindow )
    {
        // fire dragExit on listeners of previous window
        dragExit( dtde );

        // remember to send drag enter later
        bSendDragEnter = sal_True;
    }

    sal_Int32 nListeners = 0;

    // send drop event to the child window
    if( pChildWindow )
    {
        OClearableGuard aGuard2( Application::GetSolarMutex() );

        // query DropTarget from child window
        Reference< XDropTarget > xDropTarget = pChildWindow->GetDropTarget();

        if( xDropTarget.is() )
        {
            // retrieve relative mouse position
            Point relLoc = pChildWindow->ImplFrameToOutput( location );
            aGuard2.clear();

            // send DragEnterEvent if other window than at last dragOver
            if( bSendDragEnter )
            {
#if 0
                static_cast < DNDListenerContainer * > ( xDropTarget.get() )->
                    fireDragEnterEvent( static_cast < XDropTargetDragContext * > (dtde.Context.get()),
                        dtde.DropAction, ::com::sun::star::awt::Point( relLoc.X(), relLoc.Y() ),
                        dtde.SourceActions );
#endif
            }

            // fire...Event returns the number of listeners found
            nListeners = static_cast < DNDListenerContainer * > ( xDropTarget.get() )->
                fireDropEvent( dtde.Context, dtde.DropAction,
                    ::com::sun::star::awt::Point( relLoc.X(), relLoc.Y() ),
                    dtde.SourceActions, dtde.Transferable );
        }
    }

    // reject drop if no listeners found
    if( nListeners == 0 ) {
        OSL_TRACE( "rejecting drop due to missing listeners." );
        dtde.Context->rejectDrop();
    }
}

//==================================================================================================
// DNDEventDispatcher::dragEnter
//==================================================================================================

void SAL_CALL DNDEventDispatcher::dragEnter( const DropTargetDragEnterEvent& dtdee )
    throw(RuntimeException)
{
    sal_Int32 nListeners;

    // find the window that is toplevel for this coordinates
    OClearableGuard aGuard( Application::GetSolarMutex() );
    Window * pChildWindow = m_pTopWindow->ImplFindWindow( Point( dtdee.Location.X, dtdee.Location.Y ) );
    aGuard.clear();

    // assume pointer write operation to be atomic
    m_pCurrentWindow = pChildWindow;

    // fire dragEnter on listeners of current window
    nListeners = dragAction( 1, pChildWindow, dtdee );

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
    Window * pChildWindow = m_pCurrentWindow;

    // send the last window a drag exit
    if( pChildWindow )
    {
        OClearableGuard aGuard( Application::GetSolarMutex() );

        // query DropTarget from child window
        Reference< XDropTarget > xDropTarget = pChildWindow->GetDropTarget();
        aGuard.clear();

        if( xDropTarget.is() )
            static_cast < DNDListenerContainer * > ( xDropTarget.get() )->fireDragExitEvent();
    }

    // reset current window
    m_pCurrentWindow = NULL;
}

//==================================================================================================
// DNDEventDispatcher::dragOver
//==================================================================================================

void SAL_CALL DNDEventDispatcher::dragOver( const DropTargetDragEvent& dtde )
    throw(RuntimeException)
{
    sal_Int32 nListeners;

    // find the window that is toplevel for this coordinates
    OClearableGuard aGuard( Application::GetSolarMutex() );
    Window * pChildWindow = m_pTopWindow->ImplFindWindow( Point( dtde.Location.X, dtde.Location.Y ) );
    aGuard.clear();

    if( pChildWindow != m_pCurrentWindow )
    {
        // fire dragExit on listeners of previous window
        dragExit( dtde );

        // remember new window
        m_pCurrentWindow = pChildWindow;

        // fire dragEnter on listeners of current window
        nListeners = dragAction( 1, pChildWindow, dtde );
    }
    else
    {
        // fire dragOver on listeners of current window
        nListeners = dragAction( 2, pChildWindow, dtde );
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
    sal_Int32 nListeners;

    // find the window that is toplevel for this coordinates
    OClearableGuard aGuard( Application::GetSolarMutex() );
    Window * pChildWindow = m_pTopWindow->ImplFindWindow( Point( dtde.Location.X, dtde.Location.Y ) );
    aGuard.clear();

    if( pChildWindow != m_pCurrentWindow )
    {
        // fire dragExit on listeners of previous window
        dragExit( dtde );

        // remember new window
        m_pCurrentWindow = pChildWindow;

        // fire dragEnter on listeners of current window
        nListeners = dragAction( 1, pChildWindow, dtde );
    }
    else
    {
        // fire dropActionChanged on listeners of current window
        nListeners = dragAction( 3, pChildWindow, dtde );
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
// DNDEventDispatcher::dragAction
//==================================================================================================

sal_Int32 DNDEventDispatcher::dragAction( sal_Int8 action, Window * pWindow, const DropTargetDragEvent& dtde )
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
            Point relLoc = pWindow->ImplFrameToOutput( Point( dtde.Location.X, dtde.Location.Y ) );
            aGuard.clear();

            // fire...Event returns the number of listeners found
            switch( action )
            {
            case 1:
#if 0
                n = static_cast < DNDListenerContainer * > ( xDropTarget.get() )->
                    fireDragEnterEvent( dtde.Context, dtde.DropAction,
                        ::com::sun::star::awt::Point( relLoc.X(), relLoc.Y() ),
                        dtde.SourceActions );
#endif
                break;

            case 2:
                n = static_cast < DNDListenerContainer * > ( xDropTarget.get() )->
                    fireDragOverEvent( dtde.Context, dtde.DropAction,
                        ::com::sun::star::awt::Point( relLoc.X(), relLoc.Y() ),
                        dtde.SourceActions );
                break;

            case 3:
                n = static_cast < DNDListenerContainer * > ( xDropTarget.get() )->
                    fireDropActionChangedEvent( dtde.Context, dtde.DropAction,
                        ::com::sun::star::awt::Point( relLoc.X(), relLoc.Y() ),
                        dtde.SourceActions );
                break;

            default:
                ;
            }
        }
    }

    return n;
}
