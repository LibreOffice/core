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


#define WM_REGISTERDRAGDROP WM_USER + 1
#define WM_REVOKEDRAGDROP WM_USER + 2
extern Reference< XTransferable > g_XTransferable;

DWORD WINAPI DndTargetOleSTAFunc(LPVOID pParams);

DropTarget::DropTarget( const Reference<XComponentContext>& rxContext):
    WeakComponentImplHelper3<XInitialization,XDropTarget, XServiceInfo>(m_mutex),
    m_hWnd( NULL),
    m_threadIdWindow(0),
    m_threadIdTarget(0),
    m_hOleThread(0),
    m_oleThreadId( 0),
    m_pDropTarget( NULL),
    m_xContext( rxContext ),
    m_bActive(sal_True),
    m_nDefaultActions(ACTION_COPY|ACTION_MOVE|ACTION_LINK|ACTION_DEFAULT),
    m_nCurrentDropAction( ACTION_NONE),
    m_nLastDropAction(0),
    m_bDropComplete(false)
{
}

DropTarget::~DropTarget()
{
}







void SAL_CALL DropTarget::disposing()
{
    if( m_threadIdTarget)
    {
        
        PostThreadMessage( m_threadIdTarget, WM_REVOKEDRAGDROP, (WPARAM)this, 0);
        WaitForSingleObject( m_hOleThread, INFINITE);
        CloseHandle( m_hOleThread);
        //OSL_ENSURE( SUCCEEDED( hr), "HWND not valid!" );
    }
    else
    {
        RevokeDragDrop( m_hWnd);
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
    
    
    
    
    
    

    
    
    
    
    
    
    
    

    if( aArguments.getLength() > 0)
    {
        
        m_hWnd= *(HWND*)aArguments[0].getValue();
        OSL_ASSERT( IsWindow( m_hWnd) );

        
        m_threadIdWindow= GetWindowThreadProcessId( m_hWnd, NULL);

        HRESULT hr= OleInitialize( NULL);

        
        if( hr == RPC_E_CHANGED_MODE || GetCurrentThreadId() != m_threadIdWindow  )
        {
            OSL_ENSURE( ! m_threadIdTarget,"initialize was called twice");
            
            m_pDropTarget= new IDropTargetImpl( *static_cast<DropTarget*>( this) );
            m_pDropTarget->AddRef();


            
            m_threadIdWindow= GetWindowThreadProcessId( m_hWnd, NULL);
            
            
            HANDLE m_evtThreadReady= CreateEvent( NULL, FALSE, FALSE, NULL);

            m_hOleThread= CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)DndTargetOleSTAFunc,
                                            &m_evtThreadReady, 0, &m_threadIdTarget);
            WaitForSingleObject( m_evtThreadReady, INFINITE);
            CloseHandle( m_evtThreadReady);
            PostThreadMessage( m_threadIdTarget, WM_REGISTERDRAGDROP, (WPARAM)static_cast<DropTarget*>(this), 0);
        }
        else if( hr == S_OK || hr == S_FALSE)
        {
            
            
            
            if( hr == S_OK)
            {
                
                
                m_oleThreadId= CoGetCurrentProcess(); 
            }

            
            
            m_pDropTarget= new IDropTargetImpl( *static_cast<DropTarget*>( this) );
            m_pDropTarget->AddRef();
            
            if( SUCCEEDED( CoLockObjectExternal( m_pDropTarget, TRUE, FALSE)))
            {
                if( FAILED( RegisterDragDrop( m_hWnd,  m_pDropTarget) ) )
                {
                    
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




DWORD WINAPI DndTargetOleSTAFunc(LPVOID pParams)
{
    HRESULT hr= OleInitialize( NULL);
    if( SUCCEEDED( hr) )
    {
        MSG msg;
        
        PeekMessage( &msg, (HWND)NULL, 0, 0, PM_NOREMOVE);
        
        
        SetEvent( *(HANDLE*) pParams);
        
        
        DWORD threadId= GetCurrentThreadId();
        
        
        while( GetMessage(&msg, (HWND)NULL, 0, 0) )
        {
            if( msg.message == WM_REGISTERDRAGDROP)
            {
                DropTarget *pTarget= (DropTarget*)msg.wParam;
                
                
                
                AttachThreadInput( threadId , pTarget->m_threadIdWindow, TRUE );

                if( SUCCEEDED( CoLockObjectExternal(pTarget-> m_pDropTarget, TRUE, FALSE)))
                {
                    if( FAILED( RegisterDragDrop( pTarget-> m_hWnd, pTarget-> m_pDropTarget) ) )
                    {
                        
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


OUString SAL_CALL DropTarget::getImplementationName(  ) throw (RuntimeException)
{
    return OUString(DNDTARGET_IMPL_NAME);
}

sal_Bool SAL_CALL DropTarget::supportsService( const OUString& ServiceName ) throw (RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL DropTarget::getSupportedServiceNames(  ) throw (RuntimeException)
{
    OUString names[1]= {OUString(DNDTARGET_SERVICE_NAME)};
    return Sequence<OUString>(names, 1);
}


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
        
        m_nCurrentDropAction= getFilteredActions( grfKeyState, *pdwEffect);
        
        //XDropTargetDragContext::acceptDrag and specifies an action then pdwEffect
        
        m_nLastDropAction= ACTION_DEFAULT | ACTION_MOVE;

        m_currentDragContext= static_cast<XDropTargetDragContext*>( new TargetDragContext(
            static_cast<DropTarget*>(this) ) );

        //--> TRA

        
        if ( g_XTransferable.is( ) )
            m_currentData = g_XTransferable;
        else
        {
            
            m_currentData= m_aDataConverter.createTransferableFromDataObj(
                                            m_xContext, IDataObjectPtr(pDataObj));
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

            
            
            
            
            
            if( m_nLastDropAction != m_nCurrentDropAction)
                fire_dropActionChanged( e);

            
            fire_dragOver( e);
            
            
            
            
            
            
            
            sal_Int8 allowedActions= dndOleDropEffectsToActions( *pdwEffect);
            
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








inline sal_Int8 DropTarget::getFilteredActions( DWORD grfKeyState, DWORD dwEffect)
{
    sal_Int8 actions= dndOleKeysToAction( grfKeyState, dndOleDropEffectsToActions( dwEffect));
    return actions &  m_nDefaultActions;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
