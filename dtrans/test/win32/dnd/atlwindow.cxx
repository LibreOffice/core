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

     // Remove the subclass from the edit control.
    ::SetWindowLong(m_hwndEdit, GWL_WNDPROC,
                (LONG) wpOrigEditProc);

    return 0;
}

LRESULT AWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // Prepare the EDIT control
    m_hwndEdit = CreateWindowA(
        "EDIT",     // predefined class
        NULL,       // no window title
        WS_CHILD | WS_VISIBLE | WS_VSCROLL |
            ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
        0, 0, 0, 0, // set size in WM_SIZE message
        m_hWnd,       // parent window
        (HMENU) NULL, // edit control ID
        (HINSTANCE) GetWindowLong( GWL_HINSTANCE),
        NULL);

    // the map is used in the window procedure for the edit window to associate the
    // it to the right main window ( AWindow)
    mapEditToMainWnd[m_hwndEdit]= m_hWnd;
    // Superclass the edit window, because we want to process mouse messages
    wpOrigEditProc = (WNDPROC) ::SetWindowLongA(m_hwndEdit,
                GWL_WNDPROC, (LONG) EditSubclassProc);

    // Add text to the window.
    if( m_isMTA)
        ::SendMessageA(m_hwndEdit, WM_SETTEXT, 0, (LPARAM) szMTAWin);
    else
        ::SendMessageA(m_hwndEdit, WM_SETTEXT, 0, (LPARAM) szSTAWin);

    // create the DragSource
    Reference< XInterface> xint= MultiServiceFactory->createInstance("com.sun.star.datatransfer.dnd.OleDragSource");
    m_xDragSource.set( xint, UNO_QUERY );
    Reference<XInitialization> xInit( xint, UNO_QUERY);

    Any ar[2];
    ar[1]<<= (sal_uInt32)m_hWnd;
    xInit->initialize( Sequence<Any>( ar, 2) );

    //create the DropTarget
    Reference< XInterface> xintTarget= MultiServiceFactory->createInstance("com.sun.star.datatransfer.dnd.OleDropTarget");
    m_xDropTarget.set( xintTarget, UNO_QUERY );
    Reference<XInitialization> xInitTarget( xintTarget, UNO_QUERY);

    Any any;
    any <<= (sal_uInt32)m_hWnd;
    xInitTarget->initialize( Sequence<Any>( &any, 1) );

    m_xDropTarget->addDropTargetListener( static_cast<XDropTargetListener*>
        ( new DropTargetListener( m_hwndEdit)) );
//  // make this window tho a drop target
    m_xDropTarget->setActive(sal_True);

    return 0;
}

// When the mouse is dragged for a second than a drag is initiated
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
    USES_CONVERSION;
    KillTimer( 1);
    if(m_xDragSource.is())
    {

        //Get the Text out of the Edit window
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

            pData->SetData( &format,  &medium, TRUE); // releases HGLOBAL eventually

            Reference<XTransferable> xTrans= m_aDataConverter.createTransferableFromDataObj(
                                                MultiServiceFactory, pData);

            // call XDragSource::executeDrag from an MTA
            if( m_isMTA )
            {
                DWORD mtaThreadId;
                ThreadData data;
                data.source= m_xDragSource;
                data.transferable= xTrans;

                data.evtThreadReady= CreateEvent( NULL, FALSE, FALSE, NULL);

                HANDLE hThread= CreateThread( NULL, 0, MTAFunc, &data, 0, &mtaThreadId);
                // We must wait until the thread copied the ThreadData structure
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
    // Make the edit control the size of the window's
    // client area.
    ::MoveWindow(m_hwndEdit,
        0, 0,           // starting x- and y-coordinates
        LOWORD(lParam), // width of client area
        HIWORD(lParam), // height of client area
        TRUE);          // repaint window

    return 0;
}
LRESULT AWindow::OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ::SetFocus(m_hwndEdit);
    return 0;
}

// Subclass procedure for EDIT window
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
