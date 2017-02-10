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
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/any.hxx>

#include <process.h>
#include <memory>

#include "source.hxx"
#include "globals.hxx"
#include "sourcecontext.hxx"
#include "../../inc/DtObjFactory.hxx"
#include <rtl/ustring.h>
#include <osl/thread.h>
#include <winuser.h>
#include <stdio.h>

using namespace cppu;
using namespace osl;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer::dnd::DNDConstants;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt::MouseButton;
using namespace com::sun::star::awt;
using namespace com::sun::star::lang;

unsigned __stdcall DndOleSTAFunc(LPVOID pParams);

DragSource::DragSource( const Reference<XComponentContext>& rxContext):
    WeakComponentImplHelper< XDragSource, XInitialization, XServiceInfo >(m_mutex),
    m_xContext( rxContext ),
//  m_pcurrentContext_impl(0),
    m_hAppWindow(nullptr),
    m_MouseButton(0),
    m_RunningDndOperationCount(0)
{
}

DragSource::~DragSource()
{
}

/** First start a new drag and drop thread if
     the last one has finished

     ????
          Do we really need a separate thread for
          every Dnd opeartion or only if the source
          thread is an MTA thread
     ????
*/
void DragSource::StartDragImpl(
    const DragGestureEvent& trigger,
    sal_Int8 sourceActions,
    sal_Int32 /*cursor*/,
    sal_Int32 /*image*/,
    const Reference<XTransferable >& trans,
    const Reference<XDragSourceListener >& listener )
{
    // The actions supported by the drag source
    m_sourceActions= sourceActions;
    // We need to know which mouse button triggered the operation.
    // If it was the left one, then the drop occurs when that button
    // has been released and if it was the right one then the drop
    // occurs when the right button has been released. If the event is not
    // set then we assume that the left button is pressed.
    MouseEvent evtMouse;
    trigger.Event >>= evtMouse;
    m_MouseButton= evtMouse.Buttons;

    // The SourceContext class administers the XDragSourceListener s and
    // fires events to them. An instance only exists in the scope of this
    // function. However, the drag and drop operation causes callbacks
    // to the IDropSource interface implemented in this class (but only
    // while this function executes). The source context is also used
    // in DragSource::QueryContinueDrag.
    m_currentContext= static_cast<XDragSourceContext*>( new SourceContext(
                      this, listener ) );

    // Convert the XTransferable data object into an IDataObject object;

    //--> TRA
    g_XTransferable = trans;
    //<-- TRA

    m_spDataObject= CDTransObjFactory::createDataObjFromTransferable(
                    m_xContext, trans);

    // Obtain the id of the thread that created the window
    DWORD processId;
    m_threadIdWindow= GetWindowThreadProcessId( m_hAppWindow, &processId);

    // hold the instance for the DnD thread, it's to late
    // to acquire at the start of the thread procedure
    // the thread procedure is responsible for the release
    acquire();

    // The thread accesses members of this instance but does not call acquire.
    // Hopefully this instance is not destroyed before the thread has terminated.
    unsigned threadId;
    HANDLE hThread= reinterpret_cast<HANDLE>(_beginthreadex(
        nullptr, 0, DndOleSTAFunc, this, 0, &threadId));

    // detach from thread
    CloseHandle(hThread);
}

// XInitialization
/** aArguments contains a machine id */
void SAL_CALL DragSource::initialize( const Sequence< Any >& aArguments )
{
    if( aArguments.getLength() >=2)
        m_hAppWindow= reinterpret_cast<HWND>(static_cast<sal_uIntPtr>(*o3tl::doAccess<sal_uInt64>(aArguments[1])));
    OSL_ASSERT( IsWindow( m_hAppWindow) );
}

/** XDragSource */
sal_Bool SAL_CALL DragSource::isDragImageSupported(  )
{
    return false;
}

sal_Int32 SAL_CALL DragSource::getDefaultCursor( sal_Int8 /*dragAction*/ )
{
    return 0;
}

/** Notifies the XDragSourceListener by
     calling dragDropEnd */
void SAL_CALL DragSource::startDrag(
    const DragGestureEvent& trigger,
    sal_Int8 sourceActions,
    sal_Int32 cursor,
    sal_Int32 image,
    const Reference<XTransferable >& trans,
    const Reference<XDragSourceListener >& listener )
{
    // Allow only one running dnd operation at a time,
    // see XDragSource documentation

    long cnt = InterlockedIncrement(&m_RunningDndOperationCount);

    if (1 == cnt)
    {
        StartDragImpl(trigger, sourceActions, cursor, image, trans, listener);
    }
    else
    {
        cnt = InterlockedDecrement(&m_RunningDndOperationCount);

        DragSourceDropEvent dsde;

        dsde.DropAction  = ACTION_NONE;
        dsde.DropSuccess = false;

        try
        {
            listener->dragDropEnd(dsde);
        }
        catch(RuntimeException&)
        {
            OSL_FAIL("Runtime exception during event dispatching");
        }
    }
}

/** IDropTarget */
HRESULT STDMETHODCALLTYPE DragSource::QueryInterface( REFIID riid, void  **ppvObject)
{
    if( !ppvObject)
        return E_POINTER;
    *ppvObject= nullptr;

    if(  riid == __uuidof( IUnknown) )
        *ppvObject= static_cast<IUnknown*>( this);
    else if ( riid == __uuidof( IDropSource) )
        *ppvObject= static_cast<IDropSource*>( this);

    if(*ppvObject)
    {
        AddRef();
        return S_OK;
    }
    else
        return E_NOINTERFACE;

}

ULONG STDMETHODCALLTYPE DragSource::AddRef()
{
    acquire();
    return (ULONG) m_refCount;
}

ULONG STDMETHODCALLTYPE DragSource::Release()
{
    ULONG ref= m_refCount;
    release();
    return --ref;
}

/** IDropSource */
HRESULT STDMETHODCALLTYPE DragSource::QueryContinueDrag(
/* [in] */ BOOL fEscapePressed,
/* [in] */ DWORD grfKeyState)
{
#if defined DBG_CONSOLE_OUT
    printf("\nDragSource::QueryContinueDrag");
#endif

    HRESULT retVal= S_OK; // default continue DnD

    if (fEscapePressed)
    {
        retVal= DRAGDROP_S_CANCEL;
    }
    else
    {
        if( ( m_MouseButton == MouseButton::RIGHT &&  !(grfKeyState & MK_RBUTTON) ) ||
            ( m_MouseButton == MouseButton::MIDDLE && !(grfKeyState & MK_MBUTTON) ) ||
            ( m_MouseButton == MouseButton::LEFT && !(grfKeyState & MK_LBUTTON) )   ||
            ( m_MouseButton == 0 && !(grfKeyState & MK_LBUTTON) ) )
        {
            retVal= DRAGDROP_S_DROP;
        }
    }

    // fire dropActionChanged event.
    // this is actually done by the context, which also detects whether the action
    // changed at all
    sal_Int8 dropAction= fEscapePressed ? ACTION_NONE :
                                                         dndOleKeysToAction( grfKeyState, m_sourceActions);

    sal_Int8 userAction= fEscapePressed ? ACTION_NONE :
                                                         dndOleKeysToAction( grfKeyState, -1 );

    static_cast<SourceContext*>(m_currentContext.get())->fire_dropActionChanged(
        dropAction, userAction);

    return retVal;
}

HRESULT STDMETHODCALLTYPE DragSource::GiveFeedback(
/* [in] */ DWORD
#if defined DBG_CONSOLE_OUT
dwEffect
#endif
)
{
#if defined DBG_CONSOLE_OUT
    printf("\nDragSource::GiveFeedback %d", dwEffect);
#endif

    return DRAGDROP_S_USEDEFAULTCURSORS;
}

// XServiceInfo
OUString SAL_CALL DragSource::getImplementationName(  )
{
    return OUString(DNDSOURCE_IMPL_NAME);
}
// XServiceInfo
sal_Bool SAL_CALL DragSource::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL DragSource::getSupportedServiceNames(  )
{
    OUString names[1]= {OUString(DNDSOURCE_SERVICE_NAME)};

    return Sequence<OUString>(names, 1);
}

/** This function is called as extra thread from
    DragSource::executeDrag. The function
    carries out a drag and drop operation by calling
    DoDragDrop. The thread also notifies all
    XSourceListener. */
unsigned __stdcall DndOleSTAFunc(LPVOID pParams)
{
    osl_setThreadName("DragSource DndOleSTAFunc");

    // The structure contains all arguments for DoDragDrop and other
    DragSource *pSource= static_cast<DragSource*>(pParams);

    // Drag and drop only works in a thread in which OleInitialize is called.
    HRESULT hr= OleInitialize( nullptr);

    if(SUCCEEDED(hr))
    {
        // We force the creation of a thread message queue. This is necessary
        // for a later call to AttachThreadInput
        MSG msgtemp;
        PeekMessage( &msgtemp, nullptr, WM_USER, WM_USER, PM_NOREMOVE);

        DWORD threadId= GetCurrentThreadId();

        // This thread is attached to the thread that created the window. Hence
        // this thread also receives all mouse and keyboard messages which are
        // needed by DoDragDrop
        AttachThreadInput( threadId , pSource->m_threadIdWindow, TRUE );

        DWORD dwEffect= 0;
        hr= DoDragDrop(
            pSource->m_spDataObject.get(),
            static_cast<IDropSource*>(pSource),
            dndActionsToDropEffects( pSource->m_sourceActions),
            &dwEffect);

        // #105428 detach my message queue from the other threads
        // message queue before calling fire_dragDropEnd else
        // the office may appear to hang sometimes
        AttachThreadInput( threadId, pSource->m_threadIdWindow, FALSE);

        //--> TRA
        // clear the global transferable again
        g_XTransferable.clear();
        //<-- TRA

        OSL_ENSURE( hr != E_INVALIDARG, "IDataObject impl does not contain valid data");

        //Fire event
        sal_Int8 action= hr == DRAGDROP_S_DROP ? dndOleDropEffectsToActions( dwEffect) : ACTION_NONE;

        static_cast<SourceContext*>(pSource->m_currentContext.get())->fire_dragDropEnd(
                                                        hr == DRAGDROP_S_DROP, action);

        // Destroy SourceContextslkfgj
        pSource->m_currentContext= nullptr;
        // Destroy the XTransferable wrapper
        pSource->m_spDataObject=nullptr;

        OleUninitialize();
    }

    InterlockedDecrement(&pSource->m_RunningDndOperationCount);

    // the DragSource was manually acquired by
    // thread starting method DelayedStartDrag
    pSource->release();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
