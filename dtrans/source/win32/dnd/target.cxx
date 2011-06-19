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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <rtl/unload.h>

#include <stdio.h>
#include "target.hxx"
#include "idroptarget.hxx"
#include "globals.hxx"
#include "targetdropcontext.hxx"
#include "targetdragcontext.hxx"
#include <rtl/ustring.h>
using namespace cppu;
using namespace osl;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer::dnd::DNDConstants;

using ::rtl::OUString;

#define WM_REGISTERDRAGDROP WM_USER + 1
#define WM_REVOKEDRAGDROP WM_USER + 2
//--> TRA
extern Reference< XTransferable > g_XTransferable;

//<-- TRA

extern rtl_StandardModuleCount g_moduleCount;
DWORD WINAPI DndTargetOleSTAFunc(LPVOID pParams);

DropTarget::DropTarget( const Reference<XMultiServiceFactory>& sf):
    m_hWnd( NULL),
    m_serviceFactory( sf),
    WeakComponentImplHelper3<XInitialization,XDropTarget, XServiceInfo>(m_mutex),
    m_bActive(sal_True),
    m_nDefaultActions(ACTION_COPY|ACTION_MOVE|ACTION_LINK|ACTION_DEFAULT),
    m_nCurrentDropAction( ACTION_NONE),
    m_oleThreadId( 0),
    m_pDropTarget( NULL),
    m_threadIdWindow(0),
    m_threadIdTarget(0),
    m_hOleThread(0),
    m_nLastDropAction(0)


{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
}


DropTarget::~DropTarget()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );

}
// called from WeakComponentImplHelperX::dispose
// WeakComponentImplHelper calls disposing before it destroys
// itself.
// NOTE: RevokeDragDrop decrements the ref count on the IDropTarget
// interface. (m_pDropTarget)
// If the HWND is invalid then it doesn't decrement and
// the IDropTarget object will live on. MEMORY LEAK
void SAL_CALL DropTarget::disposing()
{
    HRESULT hr= S_OK;
    if( m_threadIdTarget)
    {
        // Call RevokeDragDrop and wait for the OLE thread to die;
        PostThreadMessage( m_threadIdTarget, WM_REVOKEDRAGDROP, (WPARAM)this, 0);
        WaitForSingleObject( m_hOleThread, INFINITE);
        CloseHandle( m_hOleThread);
        //OSL_ENSURE( SUCCEEDED( hr), "HWND not valid!" );
    }
    else
    {
        hr= RevokeDragDrop( m_hWnd);
        m_hWnd= 0;
    }
    if( m_pDropTarget)
    {
        CoLockObjectExternal( m_pDropTarget, FALSE, TRUE);
        m_pDropTarget->Release();
    }

    if( m_oleThreadId)
    {
        if( m_oleThreadId == CoGetCurrentProcess() )
            OleUninitialize();
    }

}

void SAL_CALL DropTarget::initialize( const Sequence< Any >& aArguments )
        throw(Exception, RuntimeException)
{
    // The window must be registered for Dnd by RegisterDragDrop. We must ensure
    // that RegisterDragDrop is called from an STA ( OleInitialize) thread.
    // As long as the window is registered we need to receive OLE messages in
    // an OLE thread. That is to say, if DropTarget::initialize was called from an
    // MTA thread then we create an OLE thread in which the window is registered.
    // The thread will stay alive until aver RevokeDragDrop has been called.

    // Additionally even if RegisterDragDrop is called from an STA thread we have
    // to ensure that it is called from the same thread that created the Window
    // otherwise meesages sent during DND won't reach the windows message queue.
    // Calling AttachThreadInput first would resolve this problem but would block
    // the message queue of the calling thread. So if the current thread
    // (even if it's an STA thread) and the thread that created the window are not
    // identical we need to create a new thread as we do when the calling thread is
    // an MTA thread.

    if( aArguments.getLength() > 0)
    {
        // Get the window handle from aArgument. It is needed for RegisterDragDrop.
        m_hWnd= *(HWND*)aArguments[0].getValue();
        OSL_ASSERT( IsWindow( m_hWnd) );

        // Obtain the id of the thread that created the window
        m_threadIdWindow= GetWindowThreadProcessId( m_hWnd, NULL);

        HRESULT hr= OleInitialize( NULL);

        // Current thread is MTA or Current thread and Window thread are not identical
        if( hr == RPC_E_CHANGED_MODE || GetCurrentThreadId() != m_threadIdWindow  )
        {
            OSL_ENSURE( ! m_threadIdTarget,"initialize was called twice");
            // create the IDropTargetImplementation
            m_pDropTarget= new IDropTargetImpl( *static_cast<DropTarget*>( this) );
            m_pDropTarget->AddRef();


            // Obtain the id of the thread that created the window
            m_threadIdWindow= GetWindowThreadProcessId( m_hWnd, NULL);
            // The event is set by the thread that we will create momentarily.
            // It indicates that the thread is ready to receive messages.
            HANDLE m_evtThreadReady= CreateEvent( NULL, FALSE, FALSE, NULL);

            m_hOleThread= CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)DndTargetOleSTAFunc,
                                            &m_evtThreadReady, 0, &m_threadIdTarget);
            WaitForSingleObject( m_evtThreadReady, INFINITE);
            CloseHandle( m_evtThreadReady);
            PostThreadMessage( m_threadIdTarget, WM_REGISTERDRAGDROP, (WPARAM)static_cast<DropTarget*>(this), 0);
        }
        else if( hr == S_OK || hr == S_FALSE)
        {
            // current thread is STA
            // If OleInitialize has been called by the caller then we must not call
            // OleUninitialize
            if( hr == S_OK)
            {
                // caller did not call OleInitialize, so we call OleUninitialize
                // remember the thread that will call OleUninitialize
                m_oleThreadId= CoGetCurrentProcess(); // get a unique thread id
            }

            // Get the window handle from aArgument. It is needed for RegisterDragDrop.
            // create the IDropTargetImplementation
            m_pDropTarget= new IDropTargetImpl( *static_cast<DropTarget*>( this) );
            m_pDropTarget->AddRef();
            // CoLockObjectExternal is prescribed by the protocol. It bumps up the ref count
            if( SUCCEEDED( CoLockObjectExternal( m_pDropTarget, TRUE, FALSE)))
            {
                if( FAILED( RegisterDragDrop( m_hWnd,  m_pDropTarget) ) )
                {
                    // do clean up if drag and drop is not possible
                    CoLockObjectExternal( m_pDropTarget, FALSE, FALSE);
                    m_pDropTarget->Release();
                    m_hWnd= NULL;
                }
            }
        }
        else
            throw Exception();

    }
}

// This function is called as extra thread from DragSource::startDrag.
// The function carries out a drag and drop operation by calling
// DoDragDrop. The thread also notifies all XSourceListener.
DWORD WINAPI DndTargetOleSTAFunc(LPVOID pParams)
{
    HRESULT hr= OleInitialize( NULL);
    if( SUCCEEDED( hr) )
    {
        MSG msg;
        // force the creation of a message queue
        PeekMessage( &msg, (HWND)NULL, 0, 0, PM_NOREMOVE);
        // Signal the creator ( DropTarget::initialize) that the thread is
        // ready to receive messages.
        SetEvent( *(HANDLE*) pParams);
        // Thread id is needed for attaching this message queue to the one of the
        // thread where the window was created.
        DWORD threadId= GetCurrentThreadId();
        // We force the creation of a thread message queue. This is necessary
        // for a later call to AttachThreadInput
        while( GetMessage(&msg, (HWND)NULL, 0, 0) )
        {
            if( msg.message == WM_REGISTERDRAGDROP)
            {
                DropTarget *pTarget= (DropTarget*)msg.wParam;
                // This thread is attached to the thread that created the window. Hence
                // this thread also receives all mouse and keyboard messages which are
                // needed
                AttachThreadInput( threadId , pTarget->m_threadIdWindow, TRUE );

                if( SUCCEEDED( CoLockObjectExternal(pTarget-> m_pDropTarget, TRUE, FALSE)))
                {
                    if( FAILED( RegisterDragDrop( pTarget-> m_hWnd, pTarget-> m_pDropTarget) ) )
                    {
                        // do clean up if drag and drop is not possible
                        CoLockObjectExternal( pTarget->m_pDropTarget, FALSE, FALSE);
                        pTarget->m_pDropTarget->Release();
                        pTarget->m_hWnd= NULL;
                    }
                }
            }
            else if( msg.message == WM_REVOKEDRAGDROP)
            {
                DropTarget *pTarget= (DropTarget*)msg.wParam;
                RevokeDragDrop( pTarget-> m_hWnd);
                // Detach this thread from the window thread
                AttachThreadInput( threadId, pTarget->m_threadIdWindow, FALSE);
                pTarget->m_hWnd= 0;
                break;
            }
            TranslateMessage(  &msg);
            DispatchMessage( &msg);
        }
        OleUninitialize();
    }
    return 0;
}




// XServiceInfo
OUString SAL_CALL DropTarget::getImplementationName(  ) throw (RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(DNDTARGET_IMPL_NAME));;
}
// XServiceInfo
sal_Bool SAL_CALL DropTarget::supportsService( const OUString& ServiceName ) throw (RuntimeException)
{
    if( ServiceName.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(DNDTARGET_SERVICE_NAME ))))
        return sal_True;
    return sal_False;
}

Sequence< OUString > SAL_CALL DropTarget::getSupportedServiceNames(  ) throw (RuntimeException)
{
    OUString names[1]= {OUString(RTL_CONSTASCII_USTRINGPARAM(DNDTARGET_SERVICE_NAME))};
    return Sequence<OUString>(names, 1);
}


// XDropTarget ----------------------------------------------------------------
void SAL_CALL DropTarget::addDropTargetListener( const Reference< XDropTargetListener >& dtl )
        throw(RuntimeException)
{
    rBHelper.addListener( ::getCppuType( &dtl ), dtl );
}

void SAL_CALL DropTarget::removeDropTargetListener( const Reference< XDropTargetListener >& dtl )
        throw(RuntimeException)
{
    rBHelper.removeListener( ::getCppuType( &dtl ), dtl );
}

sal_Bool SAL_CALL DropTarget::isActive(  ) throw(RuntimeException)
{
    return m_bActive; //m_bDropTargetRegistered;
}


void SAL_CALL DropTarget::setActive( sal_Bool _b ) throw(RuntimeException)
{
    MutexGuard g(m_mutex);
    m_bActive= _b;
}


sal_Int8 SAL_CALL DropTarget::getDefaultActions(  ) throw(RuntimeException)
{
    return m_nDefaultActions;
}

void SAL_CALL DropTarget::setDefaultActions( sal_Int8 actions ) throw(RuntimeException)
{
    OSL_ENSURE( actions < 8, "No valid default actions");
    m_nDefaultActions= actions;
}


HRESULT DropTarget::DragEnter( IDataObject *pDataObj,
                                    DWORD grfKeyState,
                                    POINTL pt,
                                    DWORD  *pdwEffect)
{
#if defined DBG_CONSOLE_OUT
    printf("\nDropTarget::DragEnter state: %x effect %d", grfKeyState, *pdwEffect);
#endif
    if( m_bActive )
    {
        // Intersection of pdwEffect and the allowed actions ( setDefaultActions)
        m_nCurrentDropAction= getFilteredActions( grfKeyState, *pdwEffect);
        // m_nLastDropAction has to be set by a listener. If no listener calls
        //XDropTargetDragContext::acceptDrag and specifies an action then pdwEffect
        // will be DROPEFFECT_NONE throughout
        m_nLastDropAction= ACTION_DEFAULT | ACTION_MOVE;

        m_currentDragContext= static_cast<XDropTargetDragContext*>( new TargetDragContext(
            static_cast<DropTarget*>(this) ) );

        //--> TRA

        // shortcut
        if ( g_XTransferable.is( ) )
            m_currentData = g_XTransferable;
        else
        {
            // Convert the IDataObject to a XTransferable
            m_currentData= m_aDataConverter.createTransferableFromDataObj(
                                            m_serviceFactory, IDataObjectPtr(pDataObj));
        }

        //<-- TRA

        if( m_nCurrentDropAction != ACTION_NONE)
        {
            DropTargetDragEnterEvent e;
            e.SupportedDataFlavors= m_currentData->getTransferDataFlavors();
            e.DropAction= m_nCurrentDropAction;
            e.Source= Reference<XInterface>( static_cast<XDropTarget*>(this),UNO_QUERY);
            e.Context= m_currentDragContext;
            POINT point={ pt.x, pt.y};
            ScreenToClient( m_hWnd, &point);
            e.LocationX= point.x;
            e.LocationY= point.y;
            e.SourceActions= dndOleDropEffectsToActions( *pdwEffect);

            fire_dragEnter( e);
            // Check if the action derived from grfKeyState (m_nCurrentDropAction) or the action set
            // by the listener (m_nCurrentDropAction) is allowed by the source. Only a allowed action is set
            // in pdwEffect. The listener notification is asynchron, that is we cannot expext that the listener
            // has already reacted to the notification.
            // If there is more then one valid action which is the case when ALT or RIGHT MOUSE BUTTON is pressed
            // then getDropEffect returns DROPEFFECT_MOVE which is the default value if no other modifier is pressed.
            // On drop the target should present the user a dialog from which the user may change the action.
            sal_Int8 allowedActions= dndOleDropEffectsToActions( *pdwEffect);
            *pdwEffect= dndActionsToSingleDropEffect( m_nLastDropAction & allowedActions);
        }
        else
        {
            *pdwEffect= DROPEFFECT_NONE;
        }
    }
    return S_OK;
}

HRESULT DropTarget::DragOver( DWORD grfKeyState,
                                   POINTL pt,
                                   DWORD  *pdwEffect)
{
    if( m_bActive)
    {
        m_nCurrentDropAction= getFilteredActions( grfKeyState, *pdwEffect);

        if( m_nCurrentDropAction)
        {
            DropTargetDragEvent e;
            e.DropAction= m_nCurrentDropAction;
            e.Source= Reference<XInterface>(static_cast<XDropTarget*>(this),UNO_QUERY);
            e.Context= m_currentDragContext;
            POINT point={ pt.x, pt.y};
            ScreenToClient( m_hWnd, &point);
            e.LocationX= point.x;
            e.LocationY= point.y;
            e.SourceActions= dndOleDropEffectsToActions( *pdwEffect);

            // if grfKeyState has changed since the last DragOver then fire events.
            // A listener might change m_nCurrentDropAction by calling the
            // XDropTargetDragContext::acceptDrag function. But this is not important
            // because in the afterwards fired dragOver event the action reflects
            // grgKeyState again.
            if( m_nLastDropAction != m_nCurrentDropAction)
                fire_dropActionChanged( e);

            // The Event contains a XDropTargetDragContext implementation.
            fire_dragOver( e);
            // Check if the action derived from grfKeyState (m_nCurrentDropAction) or the action set
            // by the listener (m_nCurrentDropAction) is allowed by the source. Only a allowed action is set
            // in pdwEffect. The listener notification is asynchron, that is we cannot expext that the listener
            // has already reacted to the notification.
            // If there is more then one valid action which is the case when ALT or RIGHT MOUSE BUTTON is pressed
            // then getDropEffect returns DROPEFFECT_MOVE which is the default value if no other modifier is pressed.
            // On drop the target should present the user a dialog from which the user may change the action.
            sal_Int8 allowedActions= dndOleDropEffectsToActions( *pdwEffect);
            // set the last action to the current if listener has not changed the value yet
            *pdwEffect= dndActionsToSingleDropEffect( m_nLastDropAction & allowedActions);
        }
        else
        {
            *pdwEffect= DROPEFFECT_NONE;
        }
    }
#if defined DBG_CONSOLE_OUT
    printf("\nDropTarget::DragOver %d", *pdwEffect );
#endif
    return S_OK;
}

HRESULT DropTarget::DragLeave( void)
{
#if defined DBG_CONSOLE_OUT
    printf("\nDropTarget::DragLeave");
#endif
    if( m_bActive)
    {

        m_currentData=0;
        m_currentDragContext= 0;
        m_currentDropContext= 0;
        m_nLastDropAction= 0;

        if( m_nDefaultActions != ACTION_NONE)
        {
            DropTargetEvent e;
            e.Source=  static_cast<XDropTarget*>(this);

            fire_dragExit( e);
        }
    }
    return S_OK;
}

HRESULT DropTarget::Drop( IDataObject  * /*pDataObj*/,
                   DWORD grfKeyState,
                   POINTL pt,
                   DWORD *pdwEffect)
{
#if defined DBG_CONSOLE_OUT
    printf("\nDropTarget::Drop");
#endif
    if( m_bActive)
    {

        m_bDropComplete= sal_False;

        m_nCurrentDropAction= getFilteredActions( grfKeyState, *pdwEffect);
        m_currentDropContext= static_cast<XDropTargetDropContext*>( new TargetDropContext( static_cast<DropTarget*>(this ))  );
        if( m_nCurrentDropAction)
        {
            DropTargetDropEvent e;
            e.DropAction= m_nCurrentDropAction;
            e.Source= Reference<XInterface>( static_cast<XDropTarget*>(this), UNO_QUERY);
            e.Context= m_currentDropContext;
            POINT point={ pt.x, pt.y};
            ScreenToClient( m_hWnd, &point);
            e.LocationX= point.x;
            e.LocationY= point.y;
            e.SourceActions= dndOleDropEffectsToActions( *pdwEffect);
            e.Transferable= m_currentData;
            fire_drop( e);

            //if fire_drop returns than a listener might have modified m_nCurrentDropAction
            if( m_bDropComplete == sal_True)
            {
                sal_Int8 allowedActions= dndOleDropEffectsToActions( *pdwEffect);
                *pdwEffect= dndActionsToSingleDropEffect( m_nCurrentDropAction & allowedActions);
            }
            else
                *pdwEffect= DROPEFFECT_NONE;
        }
        else
            *pdwEffect= DROPEFFECT_NONE;

        m_currentData= 0;
        m_currentDragContext= 0;
        m_currentDropContext= 0;
        m_nLastDropAction= 0;
    }
    return S_OK;
}



void DropTarget::fire_drop( const DropTargetDropEvent& dte)
{
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( getCppuType( (Reference<XDropTargetListener>* )0 ) );
    if( pContainer)
    {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
        {
            Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));
            listener->drop( dte);
        }
    }
}

void DropTarget::fire_dragEnter( const DropTargetDragEnterEvent& e )
{
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( getCppuType( (Reference<XDropTargetListener>* )0 ) );
    if( pContainer)
    {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
        {
            Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));
            listener->dragEnter( e);
        }
    }
}

void DropTarget::fire_dragExit( const DropTargetEvent& dte )
{
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( getCppuType( (Reference<XDropTargetListener>* )0 ) );

    if( pContainer)
    {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
        {
            Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));
            listener->dragExit( dte);
        }
    }
}

void DropTarget::fire_dragOver( const DropTargetDragEvent& dtde )
{
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( getCppuType( (Reference<XDropTargetListener>* )0 ) );
    if( pContainer)
    {
        OInterfaceIteratorHelper iter( *pContainer );
        while( iter.hasMoreElements())
        {
            Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));
            listener->dragOver( dtde);
        }
    }
}

void DropTarget::fire_dropActionChanged( const DropTargetDragEvent& dtde )
{
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( getCppuType( (Reference<XDropTargetListener>* )0 ) );
    if( pContainer)
    {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
        {
            Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));
            listener->dropActionChanged( dtde);
        }
    }
}

// Non - interface functions ============================================================
// DropTarget fires events to XDropTargetListeners. The event object contains an
// XDropTargetDropContext implementaion. When the listener calls on that interface
// then the calls are delegated from DropContext (XDropTargetDropContext) to these
// functions.
// Only one listener which visible area is affected is allowed to call on
// XDropTargetDropContext
// Returning sal_False would cause the XDropTargetDropContext or ..DragContext implementation
// to throw an InvalidDNDOperationException, meaning that a Drag is not currently performed.
// return sal_False results in throwing a InvalidDNDOperationException in the caller.

void DropTarget::_acceptDrop(sal_Int8 dropOperation, const Reference<XDropTargetDropContext>& context)
{
    if( context == m_currentDropContext)
    {
        m_nCurrentDropAction= dropOperation;
    }
}

void DropTarget::_rejectDrop( const Reference<XDropTargetDropContext>& context)
{
    if( context == m_currentDropContext)
    {
        m_nCurrentDropAction= ACTION_NONE;
    }
}

void DropTarget::_dropComplete(sal_Bool success, const Reference<XDropTargetDropContext>& context)
{
    if(context == m_currentDropContext)
    {
        m_bDropComplete= success;
    }
}
// --------------------------------------------------------------------------------------
// DropTarget fires events to XDropTargetListeners. The event object can contains an
// XDropTargetDragContext implementaion. When the listener calls on that interface
// then the calls are delegated from DragContext (XDropTargetDragContext) to these
// functions.
// Only one listener which visible area is affected is allowed to call on
// XDropTargetDragContext
void DropTarget::_acceptDrag( sal_Int8 dragOperation, const Reference<XDropTargetDragContext>& context)
{
    if( context == m_currentDragContext)
    {
        m_nLastDropAction= dragOperation;
    }
}

void DropTarget::_rejectDrag( const Reference<XDropTargetDragContext>& context)
{
    if(context == m_currentDragContext)
    {
        m_nLastDropAction= ACTION_NONE;
    }
}


//--------------------------------------------------------------------------------------


// This function determines the action dependend on the pressed
// key modifiers ( CTRL, SHIFT, ALT, Right Mouse Button). The result
// is then checked against the allowed actions which can be set through
// XDropTarget::setDefaultActions. Only those values which are also
// default actions are returned. If setDefaultActions has not been called
// beforehand the the default actions comprise all possible actions.
// params: grfKeyState - the modifier keys and mouse buttons currently pressed
inline sal_Int8 DropTarget::getFilteredActions( DWORD grfKeyState, DWORD dwEffect)
{
    sal_Int8 actions= dndOleKeysToAction( grfKeyState, dndOleDropEffectsToActions( dwEffect));
    return actions &  m_nDefaultActions;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
