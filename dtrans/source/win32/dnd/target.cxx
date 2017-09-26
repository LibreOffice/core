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
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/any.hxx>

#include <stdio.h>
#include "target.hxx"
#include "idroptarget.hxx"
#include "globals.hxx"
#include "targetdropcontext.hxx"
#include "targetdragcontext.hxx"
#include <rtl/ustring.h>
#include <osl/thread.h>

#include "../dtobj/DOTransferable.hxx"

using namespace cppu;
using namespace osl;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer::dnd::DNDConstants;

#define WM_REGISTERDRAGDROP WM_USER + 1
#define WM_REVOKEDRAGDROP WM_USER + 2

DWORD WINAPI DndTargetOleSTAFunc(LPVOID pParams);

DropTarget::DropTarget( const Reference<XComponentContext>& rxContext):
    WeakComponentImplHelper<XInitialization,XDropTarget, XServiceInfo>(m_mutex),
    m_hWnd( nullptr),
    m_threadIdWindow(0),
    m_threadIdTarget(0),
    m_hOleThread(nullptr),
    m_oleThreadId( 0),
    m_pDropTarget( nullptr),
    m_xContext( rxContext ),
    m_bActive(true),
    m_nDefaultActions(ACTION_COPY|ACTION_MOVE|ACTION_LINK|ACTION_DEFAULT),
    m_nCurrentDropAction( ACTION_NONE),
    m_nLastDropAction(0),
    m_bDropComplete(false)
{
}

DropTarget::~DropTarget()
{
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
    if( m_threadIdTarget)
    {
        // Call RevokeDragDrop and wait for the OLE thread to die;
        PostThreadMessageW( m_threadIdTarget, WM_REVOKEDRAGDROP, reinterpret_cast<WPARAM>(this), 0);
        WaitForSingleObject( m_hOleThread, INFINITE);
        CloseHandle( m_hOleThread);
        //OSL_ENSURE( SUCCEEDED( hr), "HWND not valid!" );
    }
    else
    {
        RevokeDragDrop( m_hWnd);
        m_hWnd= nullptr;
    }
    if( m_pDropTarget)
    {
        CoLockObjectExternal( m_pDropTarget, FALSE, TRUE);
        m_pDropTarget->Release();
        m_pDropTarget = nullptr;
    }

    if( m_oleThreadId)
    {
        if( m_oleThreadId == CoGetCurrentProcess() )
            OleUninitialize();
    }

}

void SAL_CALL DropTarget::initialize( const Sequence< Any >& aArguments )
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
        m_hWnd= reinterpret_cast<HWND>(static_cast<sal_uIntPtr>(*o3tl::doAccess<sal_uInt64>(aArguments[0])));
        OSL_ASSERT( IsWindow( m_hWnd) );

        // Obtain the id of the thread that created the window
        m_threadIdWindow= GetWindowThreadProcessId( m_hWnd, nullptr);

        HRESULT hr= OleInitialize( nullptr);

        // Current thread is MTA or Current thread and Window thread are not identical
        if( hr == RPC_E_CHANGED_MODE || GetCurrentThreadId() != m_threadIdWindow  )
        {
            OSL_ENSURE( ! m_threadIdTarget,"initialize was called twice");
            // create the IDropTargetImplementation
            m_pDropTarget= new IDropTargetImpl( *this );
            m_pDropTarget->AddRef();

            // Obtain the id of the thread that created the window
            m_threadIdWindow= GetWindowThreadProcessId( m_hWnd, nullptr);
            // The event is set by the thread that we will create momentarily.
            // It indicates that the thread is ready to receive messages.
            HANDLE m_evtThreadReady= CreateEventW( nullptr, FALSE, FALSE, nullptr);

            m_hOleThread= CreateThread( nullptr, 0, DndTargetOleSTAFunc,
                                            &m_evtThreadReady, 0, &m_threadIdTarget);
            WaitForSingleObject( m_evtThreadReady, INFINITE);
            CloseHandle( m_evtThreadReady);
            PostThreadMessageW( m_threadIdTarget, WM_REGISTERDRAGDROP, reinterpret_cast<WPARAM>(this), 0);
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
            m_pDropTarget= new IDropTargetImpl( *this );
            m_pDropTarget->AddRef();
            // CoLockObjectExternal is prescribed by the protocol. It bumps up the ref count
            if( SUCCEEDED( CoLockObjectExternal( m_pDropTarget, TRUE, FALSE)))
            {
                if( FAILED( RegisterDragDrop( m_hWnd,  m_pDropTarget) ) )
                {
                    // do clean up if drag and drop is not possible
                    CoLockObjectExternal( m_pDropTarget, FALSE, FALSE);
                    m_pDropTarget->Release();
                    m_pDropTarget = nullptr;
                    m_hWnd= nullptr;
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
    osl_setThreadName("DropTarget DndTargetOleSTAFunc");

    HRESULT hr= OleInitialize( nullptr);
    if( SUCCEEDED( hr) )
    {
        MSG msg;
        // force the creation of a message queue
        PeekMessageW( &msg, nullptr, 0, 0, PM_NOREMOVE);
        // Signal the creator ( DropTarget::initialize) that the thread is
        // ready to receive messages.
        SetEvent( *static_cast<HANDLE*>(pParams));
        // Thread id is needed for attaching this message queue to the one of the
        // thread where the window was created.
        DWORD threadId= GetCurrentThreadId();
        // We force the creation of a thread message queue. This is necessary
        // for a later call to AttachThreadInput
        while( GetMessageW(&msg, nullptr, 0, 0) )
        {
            if( msg.message == WM_REGISTERDRAGDROP)
            {
                DropTarget *pTarget= reinterpret_cast<DropTarget*>(msg.wParam);
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
                        pTarget->m_pDropTarget = nullptr;
                        pTarget->m_hWnd= nullptr;
                    }
                }
            }
            else if( msg.message == WM_REVOKEDRAGDROP)
            {
                DropTarget *pTarget= reinterpret_cast<DropTarget*>(msg.wParam);
                RevokeDragDrop( pTarget-> m_hWnd);
                // Detach this thread from the window thread
                AttachThreadInput( threadId, pTarget->m_threadIdWindow, FALSE);
                pTarget->m_hWnd= nullptr;
                break;
            }
            TranslateMessage(  &msg);
            DispatchMessageW( &msg);
        }
        OleUninitialize();
    }
    return 0;
}

// XServiceInfo
OUString SAL_CALL DropTarget::getImplementationName(  )
{
    return OUString(DNDTARGET_IMPL_NAME);
}
// XServiceInfo
sal_Bool SAL_CALL DropTarget::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL DropTarget::getSupportedServiceNames(  )
{
    OUString names[1]= {OUString(DNDTARGET_SERVICE_NAME)};
    return Sequence<OUString>(names, 1);
}

// XDropTarget
void SAL_CALL DropTarget::addDropTargetListener( const Reference< XDropTargetListener >& dtl )
{
    rBHelper.addListener( cppu::UnoType<decltype(dtl)>::get(), dtl );
}

void SAL_CALL DropTarget::removeDropTargetListener( const Reference< XDropTargetListener >& dtl )
{
    rBHelper.removeListener( cppu::UnoType<decltype(dtl)>::get(), dtl );
}

sal_Bool SAL_CALL DropTarget::isActive(  )
{
    return m_bActive; //m_bDropTargetRegistered;
}

void SAL_CALL DropTarget::setActive( sal_Bool _b )
{
    MutexGuard g(m_mutex);
    m_bActive= _b;
}

sal_Int8 SAL_CALL DropTarget::getDefaultActions(  )
{
    return m_nDefaultActions;
}

void SAL_CALL DropTarget::setDefaultActions( sal_Int8 actions )
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
            this ) );

        //--> TRA

        // shortcut
        if ( g_XTransferable.is( ) )
            m_currentData = g_XTransferable;
        else
        {
            // Convert the IDataObject to a XTransferable
            m_currentData= CDOTransferable::create(
                                            m_xContext, IDataObjectPtr(pDataObj));
        }

        //<-- TRA

        if( m_nCurrentDropAction != ACTION_NONE)
        {
            DropTargetDragEnterEvent e;
            e.SupportedDataFlavors= m_currentData->getTransferDataFlavors();
            e.DropAction= m_nCurrentDropAction;
            e.Source.set( static_cast<XDropTarget*>(this),UNO_QUERY);
            e.Context= m_currentDragContext;
            POINT point={ pt.x, pt.y};
            ScreenToClient( m_hWnd, &point);
            e.LocationX= point.x;
            e.LocationY= point.y;
            e.SourceActions= dndOleDropEffectsToActions( *pdwEffect);

            fire_dragEnter( e);
            // Check if the action derived from grfKeyState (m_nCurrentDropAction) or the action set
            // by the listener (m_nCurrentDropAction) is allowed by the source. Only a allowed action is set
            // in pdwEffect. The listener notification is asynchron, that is we cannot expect that the listener
            // has already reacted to the notification.
            // If there is more than one valid action which is the case when ALT or RIGHT MOUSE BUTTON is pressed
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
            e.Source.set(static_cast<XDropTarget*>(this),UNO_QUERY);
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
            // in pdwEffect. The listener notification is asynchron, that is we cannot expect that the listener
            // has already reacted to the notification.
            // If there is more than one valid action which is the case when ALT or RIGHT MOUSE BUTTON is pressed
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

HRESULT DropTarget::DragLeave()
{
#if defined DBG_CONSOLE_OUT
    printf("\nDropTarget::DragLeave");
#endif
    if( m_bActive)
    {

        m_currentData=nullptr;
        m_currentDragContext= nullptr;
        m_currentDropContext= nullptr;
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

        m_bDropComplete= false;

        m_nCurrentDropAction= getFilteredActions( grfKeyState, *pdwEffect);
        m_currentDropContext= static_cast<XDropTargetDropContext*>( new TargetDropContext( this )  );
        if( m_nCurrentDropAction)
        {
            DropTargetDropEvent e;
            e.DropAction= m_nCurrentDropAction;
            e.Source.set( static_cast<XDropTarget*>(this), UNO_QUERY);
            e.Context= m_currentDropContext;
            POINT point={ pt.x, pt.y};
            ScreenToClient( m_hWnd, &point);
            e.LocationX= point.x;
            e.LocationY= point.y;
            e.SourceActions= dndOleDropEffectsToActions( *pdwEffect);
            e.Transferable= m_currentData;
            fire_drop( e);

            //if fire_drop returns than a listener might have modified m_nCurrentDropAction
            if( m_bDropComplete )
            {
                sal_Int8 allowedActions= dndOleDropEffectsToActions( *pdwEffect);
                *pdwEffect= dndActionsToSingleDropEffect( m_nCurrentDropAction & allowedActions);
            }
            else
                *pdwEffect= DROPEFFECT_NONE;
        }
        else
            *pdwEffect= DROPEFFECT_NONE;

        m_currentData= nullptr;
        m_currentDragContext= nullptr;
        m_currentDropContext= nullptr;
        m_nLastDropAction= 0;
    }
    return S_OK;
}

void DropTarget::fire_drop( const DropTargetDropEvent& dte)
{
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( cppu::UnoType<XDropTargetListener>::get());
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
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( cppu::UnoType<XDropTargetListener>::get());
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
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( cppu::UnoType<XDropTargetListener>::get());

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
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( cppu::UnoType<XDropTargetListener>::get());
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
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( cppu::UnoType<XDropTargetListener>::get());
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

// Non - interface functions
// DropTarget fires events to XDropTargetListeners. The event object contains an
// XDropTargetDropContext implementation. When the listener calls on that interface
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

void DropTarget::_dropComplete(bool success, const Reference<XDropTargetDropContext>& context)
{
    if(context == m_currentDropContext)
    {
        m_bDropComplete= success;
    }
}

// DropTarget fires events to XDropTargetListeners. The event object can contains an
// XDropTargetDragContext implementation. When the listener calls on that interface
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

// This function determines the action dependent on the pressed
// key modifiers ( CTRL, SHIFT, ALT, Right Mouse Button). The result
// is then checked against the allowed actions which can be set through
// XDropTarget::setDefaultActions. Only those values which are also
// default actions are returned. If setDefaultActions has not been called
// beforehand the default actions comprise all possible actions.
// params: grfKeyState - the modifier keys and mouse buttons currently pressed
inline sal_Int8 DropTarget::getFilteredActions( DWORD grfKeyState, DWORD dwEffect)
{
    sal_Int8 actions= dndOleKeysToAction( grfKeyState, dndOleDropEffectsToActions( dwEffect));
    return actions &  m_nDefaultActions;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
