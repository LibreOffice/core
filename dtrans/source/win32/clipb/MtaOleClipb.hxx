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

#ifndef INCLUDED_DTRANS_SOURCE_WIN32_CLIPB_MTAOLECLIPB_HXX
#define INCLUDED_DTRANS_SOURCE_WIN32_CLIPB_MTAOLECLIPB_HXX

#include <sal/types.h>
#include <osl/mutex.hxx>

#include <objidl.h>

// the Mta-Ole clipboard class is for internal use only!
// only one instance of this class should be created, the
// user has to ensure this!
// the class is not thread-safe because it will be used
// only from within the clipboard service and the methods
// of the clipboard service are already synchronized

class CMtaOleClipboard
{
public:
    typedef void ( WINAPI *LPFNC_CLIPVIEWER_CALLBACK_t )( void );

public:
    CMtaOleClipboard( );
    ~CMtaOleClipboard( );

    // clipboard functions
    HRESULT setClipboard( IDataObject* pIDataObject );
    HRESULT getClipboard( IDataObject** ppIDataObject );
    HRESULT flushClipboard( );

    // register/unregister a clipboard viewer; there can only
    // be one at a time; parameter NULL means unregister
    // a clipboard viewer
    // returns true on success else false; use GetLastError( ) in
    // false case
    bool registerClipViewer( LPFNC_CLIPVIEWER_CALLBACK_t pfncClipViewerCallback );

private:
    unsigned int run( );

    // create a hidden window which serves as an request target; so we
    // guarantee synchronization
    void createMtaOleReqWnd( );

    // message support
    bool     postMessage( UINT msg, WPARAM wParam = 0, LPARAM lParam = 0 );
    LRESULT  sendMessage( UINT msg, WPARAM wParam = 0, LPARAM lParam = 0 );

    // message handler functions; remember these functions are called
    // from a different thread context!

    static HRESULT onSetClipboard( IDataObject* pIDataObject );
    static HRESULT onGetClipboard( LPSTREAM* ppStream );
    static HRESULT onFlushClipboard( );
    bool     onRegisterClipViewer( LPFNC_CLIPVIEWER_CALLBACK_t pfncClipViewerCallback );

    // win32 clipboard listener support
    LRESULT onClipboardUpdate();

    static LRESULT CALLBACK mtaOleReqWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static unsigned int WINAPI oleThreadProc( LPVOID pParam );

    static unsigned int WINAPI clipboardChangedNotifierThreadProc( LPVOID pParam );

    bool WaitForThreadReady( ) const;

private:
    HANDLE                      m_hOleThread;
    unsigned                    m_uOleThreadId;
    HANDLE                      m_hEvtThrdReady;
    HWND                        m_hwndMtaOleReqWnd;
    HANDLE                      m_hEvtWndDisposed;
    ATOM                        m_MtaOleReqWndClassAtom;
    LPFNC_CLIPVIEWER_CALLBACK_t m_pfncClipViewerCallback;
    bool                        m_bInRegisterClipViewer;

    bool                        m_bRunClipboardNotifierThread;
    HANDLE                      m_hClipboardChangedNotifierThread;
    HANDLE                      m_hClipboardChangedNotifierEvents[2];
    HANDLE&                     m_hClipboardChangedEvent;
    HANDLE&                     m_hTerminateClipboardChangedNotifierEvent;
    osl::Mutex                  m_ClipboardChangedEventCountMutex;
    sal_Int32                   m_ClipboardChangedEventCount;

    osl::Mutex                  m_pfncClipViewerCallbackMutex;

    static CMtaOleClipboard*    s_theMtaOleClipboardInst;

// not allowed
private:
    CMtaOleClipboard( const CMtaOleClipboard& );
    CMtaOleClipboard& operator=( const CMtaOleClipboard& );

    friend LRESULT CALLBACK mtaOleReqWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
