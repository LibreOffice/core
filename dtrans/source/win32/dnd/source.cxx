/*************************************************************************
 *
 *  $RCSfile: source.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-23 09:39:59 $
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

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_DNDCONSTANTS_HPP_
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MOUSEBUTTON_HPP_
#include <com/sun/star/awt/MouseButton.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MOUSEEVENT_HPP_
#include <com/sun/star/awt/MouseEvent.hpp>
#endif


#include "source.hxx"
#include "globals.hxx"
#include "sourcecontext.hxx"
#include "../../inc/DtObjFactory.hxx"
#include <rtl/ustring.h>
#include <comdef.h>
#include <process.h>
#include <winuser.h>
#include <stdio.h>

using namespace rtl;
using namespace cppu;
using namespace osl;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer::dnd::DNDConstants;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt::MouseButton;
using namespace com::sun::star::awt;
using namespace com::sun::star::lang;


//--> TRA

extern Reference< XTransferable > g_XTransferable;

//<-- TRA

DWORD WINAPI DndOleSTAFunc(LPVOID pParams);


DragSource::DragSource( const Reference<XMultiServiceFactory>& sf):
    m_serviceFactory( sf),
    WeakComponentImplHelper2< XDragSource, XInitialization >(m_mutex),
//  m_pcurrentContext_impl(0),
    m_hAppWindow(0),
    m_MouseButton(0)
{
}

DragSource::~DragSource()
{
}

  // XInitialization

// aArguments contains a machine id
void SAL_CALL DragSource::initialize( const Sequence< Any >& aArguments )
    throw(Exception, RuntimeException)
{
    if( aArguments.getLength() >=2)
        m_hAppWindow= *(HWND*)aArguments[1].getValue();
    OSL_ASSERT( IsWindow( m_hAppWindow) );
}

// XDragSource
sal_Bool SAL_CALL DragSource::isDragImageSupported(  )
         throw(RuntimeException)
{
    return 0;
}

sal_Int32 SAL_CALL DragSource::getDefaultCursor( sal_Int8 dragAction )
          throw( IllegalArgumentException, RuntimeException)
{
    return 0;
}

// Notifies the XDragSourceListener by calling dragDropEnd
void SAL_CALL DragSource::startDrag( const DragGestureEvent& trigger,
                             sal_Int8 sourceActions,
                             sal_Int32 cursor,
                             sal_Int32 image,
                             const Reference<XTransferable >& trans,
                             const Reference<XDragSourceListener >& listener )
        throw( RuntimeException)
{
    // The actions supported by the drag source
    m_sourceActions= sourceActions;
    // We need the to know which mouse button triggered the operation.
    // If it was the left one, then the drop occurs when that button
    // has been released and if it was the right one then the drop
    // occurs when the right button has been released. If the event is not
    // set then we assume that the left button is pressed.
    MouseEvent evtMouse;
    trigger.Event >>= evtMouse;
    m_MouseButton= evtMouse.Buttons;

    // The SourceContext class administers the XDragSourceListener s and
    // fires events to them. An instance only exists in the scope of this
    // functions. However, the drag and drop operation causes callbacks
    // to the IDropSource interface implemented in this class (but only
    // while this function executes). The source context is also used
    // in DragSource::QueryContinueDrag.
    m_currentContext= static_cast<XDragSourceContext*>( new SourceContext(
                      static_cast<DragSource*>(this), listener ) );

    // Convert the XTransferable data object into an IDataObject object;

    //--> TRA
    g_XTransferable = trans;
    //<-- TRA

    m_spDataObject= m_aDataConverter.createDataObjFromTransferable(
                    m_serviceFactory, trans);

    // Obtain the id of the thread that created the window
    DWORD processId;
    m_threadIdWindow= GetWindowThreadProcessId( m_hAppWindow, &processId);

    // The thread acccesses members of this instance but does not call acquire.
    // Hopefully this instance is not destroyed before the thread has terminated.
    DWORD threadId;
    HANDLE holeThread= CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)DndOleSTAFunc,
                         static_cast<DragSource*>(this), 0, &threadId);
}

#ifdef DEBUG
void SAL_CALL DragSource::release()
{
    if( m_refCount == 1)
    {
        int a = m_refCount;
    }
    WeakComponentImplHelper2< XDragSource, XInitialization>::release();
}
#endif



//IDropTarget ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE DragSource::QueryInterface( REFIID riid, void  **ppvObject)
{
    if( !ppvObject)
        return E_POINTER;
    *ppvObject= NULL;

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

ULONG STDMETHODCALLTYPE DragSource::AddRef( void)
{
    acquire();
    return (ULONG) m_refCount;
}

ULONG STDMETHODCALLTYPE DragSource::Release( void)
{
    release();
    return (ULONG) m_refCount;
}

// IDropSource
HRESULT STDMETHODCALLTYPE DragSource::QueryContinueDrag(
/* [in] */ BOOL fEscapePressed,
/* [in] */ DWORD grfKeyState)
{
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
                  ( m_sourceActions & dndOleKeysToAction( grfKeyState));
    sal_Int8 userAction= fEscapePressed ? ACTION_NONE :
                  dndOleKeysToAction( grfKeyState);
    static_cast<SourceContext*>(m_currentContext.get())->fire_dropActionChanged( dropAction, userAction);

#if DBG_CONSOLE_OUT
    printf("\nDragSource::QueryContinueDrag");
#endif
    return retVal;
}

HRESULT STDMETHODCALLTYPE DragSource::GiveFeedback(
/* [in] */ DWORD dwEffect)
{
#if DBG_CONSOLE_OUT
    printf("\nDragSource::GiveFeedback");
#endif

    return DRAGDROP_S_USEDEFAULTCURSORS;
}



// This function is called as extra thread from DragSource::executeDrag.
// The function carries out a drag and drop operation by calling
// DoDragDrop. The thread also notifies all XSourceListener.
DWORD WINAPI DndOleSTAFunc(LPVOID pParams)
{
    // The structure contains all arguments for DoDragDrop and other
    DragSource *pSource= (DragSource*)pParams;
    // Drag and drop only works in a thread in which OleInitialize is called.

    HRESULT hr= OleInitialize( NULL);
    if( SUCCEEDED( hr) )
    {
        // We force the creation of a thread message queue. This is necessary
        // for a later call to AttachThreadInput
        MSG msgtemp;
        PeekMessage( &msgtemp, NULL, WM_USER, WM_USER, PM_NOREMOVE);

        DWORD threadId= GetCurrentThreadId();
        // This thread is attached to the thread that created the window. Hence
        // this thread also receives all mouse and keyboard messages which are
        // needed by DoDragDrop
        AttachThreadInput( threadId , pSource->m_threadIdWindow, TRUE );

#ifdef DBG_CLIPBOARD_DATA
        m_spDataObject->release();
        OleGetClipboard( &pSource->m_spDataObject);
#endif
        DWORD dwEffect= 0;
        hr= DoDragDrop(
            pSource->m_spDataObject,
            static_cast<IDropSource*>(pSource),
            dndActionsToDropEffects( pSource->m_sourceActions),
            &dwEffect);

        //--> TRA
        // clear the global transferable again
        g_XTransferable = Reference< XTransferable >( );
        //<-- TRA

        OSL_ENSURE( hr != E_INVALIDARG, "IDataObject impl does not contain valid data");

        //Fire event
        sal_Int8 action= hr == DRAGDROP_S_DROP ? dndOleDropEffectsToActions( dwEffect) : ACTION_NONE;
        static_cast<SourceContext*>(pSource->m_currentContext.get())->fire_dragDropEnd(
                                                        hr == DRAGDROP_S_DROP ? sal_True : sal_False,
                                                        action);
        // Destroy SourceContextslkfgj
        pSource->m_currentContext= 0;
        // Destroy the XTransferable wrapper
        pSource->m_spDataObject=0;

        // Detach this thread from the window thread
        AttachThreadInput( threadId, pSource->m_threadIdWindow, FALSE);
        OleUninitialize();
    }
    return 0;
}




