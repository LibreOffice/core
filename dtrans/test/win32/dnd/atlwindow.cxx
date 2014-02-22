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

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>

#include <cppuhelper/servicefactory.hxx>
#include <rtl/string.h>

#include "atlwindow.hxx"
#include "targetlistener.hxx"
#include "sourcelistener.hxx"
#include <map>

#include <winbase.h>
using namespace com::sun::star::lang;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer::dnd::DNDConstants;
using namespace cppu;
using namespace std;


LRESULT APIENTRY EditSubclassProc( HWND hwnd, UINT uMsg,WPARAM wParam, LPARAM lParam) ;


extern Reference< XMultiServiceFactory > MultiServiceFactory;
DWORD WINAPI MTAFunc(LPVOID pParams);

char* szSTAWin= "XDragSource::executeDrag is called from the same "
                "OLE STA thread that created the window.";
char* szMTAWin= "XDragSource::executeDrag is called from a MTA thread "
                "that did not create the window.";

WNDPROC wpOrigEditProc;

map<HWND, HWND> mapEditToMainWnd;

LRESULT AWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    Reference<XComponent> xcompSource( m_xDragSource, UNO_QUERY);

    PostQuitMessage(0);


    m_xDropTarget=0;
    m_xDragSource=0;


     
    ::SetWindowLong(m_hwndEdit, GWL_WNDPROC,
                (LONG) wpOrigEditProc);

    return 0;
}


LRESULT AWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    
    m_hwndEdit = CreateWindowA(
        "EDIT",     
        NULL,       
        WS_CHILD | WS_VISIBLE | WS_VSCROLL |
            ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
        0, 0, 0, 0, 
        m_hWnd,       
        (HMENU) NULL, 
        (HINSTANCE) GetWindowLong( GWL_HINSTANCE),
        NULL);

    
    
    mapEditToMainWnd[m_hwndEdit]= m_hWnd;
    
    wpOrigEditProc = (WNDPROC) ::SetWindowLongA(m_hwndEdit,
                GWL_WNDPROC, (LONG) EditSubclassProc);


    
    if( m_isMTA)
        ::SendMessageA(m_hwndEdit, WM_SETTEXT, 0, (LPARAM) szMTAWin);
    else
        ::SendMessageA(m_hwndEdit, WM_SETTEXT, 0, (LPARAM) szSTAWin);


    
    Reference< XInterface> xint= MultiServiceFactory->createInstance(OUString(L"com.sun.star.datatransfer.dnd.OleDragSource"));
    m_xDragSource= Reference<XDragSource>( xint, UNO_QUERY);
    Reference<XInitialization> xInit( xint, UNO_QUERY);

    Any ar[2];
    ar[1]<<= (sal_uInt32)m_hWnd;
    xInit->initialize( Sequence<Any>( ar, 2) );

    
    Reference< XInterface> xintTarget= MultiServiceFactory->createInstance(OUString(L"com.sun.star.datatransfer.dnd.OleDropTarget"));
    m_xDropTarget= Reference<XDropTarget>( xintTarget, UNO_QUERY);
    Reference<XInitialization> xInitTarget( xintTarget, UNO_QUERY);

    Any any;
    any <<= (sal_uInt32)m_hWnd;
    xInitTarget->initialize( Sequence<Any>( &any, 1) );


    m_xDropTarget->addDropTargetListener( static_cast<XDropTargetListener*>
        ( new DropTargetListener( m_hwndEdit)) );

    m_xDropTarget->setActive(sal_True);

    return 0;
}


LRESULT AWindow::OnMouseAction(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if( uMsg== WM_LBUTTONDOWN)
    {
        SetTimer( 1, 1000);
    }

    else if( uMsg == WM_LBUTTONUP)
    {
        KillTimer(  1);
    }

    return 0;
}

LRESULT AWindow::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr;
    USES_CONVERSION;
    KillTimer( 1);
    if(m_xDragSource.is())
    {

        
        int length= (int)::SendMessageA( m_hwndEdit, WM_GETTEXTLENGTH, 0, 0);
        char * pBuffer= new char[length + 1];
        ZeroMemory( pBuffer, length + 1);
        ::SendMessageA( m_hwndEdit, WM_GETTEXT, length, (LPARAM) pBuffer);

        IDataObject* pData= NULL;
        HRESULT hr= CreateDataCache( NULL, CLSID_NULL, __uuidof(IDataObject),(void**) &pData);
        if( pData)
        {
            FORMATETC format={ CF_TEXT, NULL, DVASPECT_CONTENT, -1, };

            HGLOBAL mem= GlobalAlloc(GHND, length + 1 );
            void* pMem= GlobalLock( mem);
            memcpy( pMem, pBuffer, length+1);
            GlobalUnlock( mem);

            STGMEDIUM medium;
            medium.tymed= TYMED_HGLOBAL;
            medium.hGlobal= mem;
            medium.pUnkForRelease= NULL;

            pData->SetData( &format,  &medium, TRUE); 

            Reference<XTransferable> xTrans= m_aDataConverter.createTransferableFromDataObj(
                                                MultiServiceFactory, pData);

            
            if( m_isMTA )
            {
                DWORD mtaThreadId;
                ThreadData data;
                data.source= m_xDragSource;
                data.transferable= xTrans;

                data.evtThreadReady= CreateEvent( NULL, FALSE, FALSE, NULL);

                HANDLE hThread= CreateThread( NULL, 0, MTAFunc, &data, 0, &mtaThreadId);
                
                WaitForSingleObject( data.evtThreadReady, INFINITE);
                CloseHandle( data.evtThreadReady);


            }
            else
            {
                m_xDragSource->startDrag( DragGestureEvent(),
                    ACTION_LINK|ACTION_MOVE|ACTION_COPY,
                    0,
                    0,
                    xTrans,
                    Reference<XDragSourceListener>( static_cast<XDragSourceListener*>(new DragSourceListener() ) ) );
            }
        }

        delete[] pBuffer;
    }

    return 0;
}

LRESULT AWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    
    
    ::MoveWindow(m_hwndEdit,
        0, 0,           
        LOWORD(lParam), 
        HIWORD(lParam), 
        TRUE);          

    return 0;
}
LRESULT AWindow::OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ::SetFocus(m_hwndEdit);
    return 0;
}




LRESULT APIENTRY EditSubclassProc( HWND hwnd, UINT uMsg,WPARAM wParam, LPARAM lParam)
{

    if( uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
    {
        HWND hAWindow= mapEditToMainWnd[hwnd];
        ::SendMessage( hAWindow, uMsg, wParam, lParam);

    }
    return CallWindowProc( wpOrigEditProc, hwnd, uMsg,
        wParam, lParam);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
