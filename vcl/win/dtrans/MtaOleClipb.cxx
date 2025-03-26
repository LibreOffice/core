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

/*
    MtaOleClipb.cxx - documentation

    This class setup a single threaded apartment (sta) thread to deal with
    the ole clipboard, which runs only in an sta thread.
    The consequence is that callback from the ole clipboard are in the
    context of this sta thread. In the soffice applications this may lead
    to problems because they all use the one and only mutex called
    SolarMutex.
    In order to transfer clipboard requests to our sta thread we use a
    hidden window and forward these requests via window messages.
*/

#include <osl/diagnose.h>
#include <sal/log.hxx>

#include "MtaOleClipb.hxx"

#include <svsys.h>
#include <win/saldata.hxx>

#include <osl/thread.h>

#include <wchar.h>
#include <process.h>

#include <systools/win32/comtools.hxx>
#include <systools/win32/retry_if_failed.hxx>
#include <systools/win32/wait_for_multiple_objects.hxx>

#include <comphelper/windowserrorstring.hxx>

namespace /* private */
{
    const wchar_t g_szWndClsName[] = L"MtaOleReqWnd###";

    // messages constants

    const sal_uInt32 MSG_SETCLIPBOARD               = WM_USER + 0x0001;
    const sal_uInt32 MSG_GETCLIPBOARD               = WM_USER + 0x0002;
    const sal_uInt32 MSG_REGCLIPVIEWER              = WM_USER + 0x0003;
    const sal_uInt32 MSG_FLUSHCLIPBOARD             = WM_USER + 0x0004;
    const sal_uInt32 MSG_SHUTDOWN                   = WM_USER + 0x0005;

    const sal_uInt32 MAX_WAITTIME                   = 10000;  // msec
    const sal_uInt32 MAX_WAIT_SHUTDOWN              = 10000; // msec

    const bool MANUAL_RESET = true;
    const bool INIT_NONSIGNALED = false;

    /*  Similar to osl conditions, with two condition objects passed to the Wait function.
    */
    class Win32Condition
    {
    public:
        Win32Condition() = default;

        ~Win32Condition() { CloseHandle(m_hEvent); }

        // wait infinite for own event (or abort event) be signaled
        // leave messages sent through
        bool wait(HANDLE hEvtAbort)
        {
            const HANDLE hWaitArray[2] = { m_hEvent, hEvtAbort };
            switch (sal::systools::WaitForMultipleObjects_COMDispatch(2, hWaitArray, INFINITE))
            {
                    case WAIT_OBJECT_0: // wait successful
                        return true;

                    case WAIT_OBJECT_0 + 1: // wait aborted
                    default: // WAIT_FAILED?
                        return false;
            }
        }

        // set the event
        void set() { SetEvent(m_hEvent); }

    private:
        HANDLE m_hEvent = CreateEventW(nullptr, MANUAL_RESET, INIT_NONSIGNALED, nullptr);

        // prevent copy/assignment
        Win32Condition(const Win32Condition&) = delete;
        Win32Condition& operator=(const Win32Condition&) = delete;
    };

    // we use one condition for every request

    struct MsgCtx
    {
        Win32Condition  aCondition;
        HRESULT         hr;
    };

CMtaOleClipboard* s_theMtaOleClipboardInst = nullptr;

// marshal an IDataObject
HRESULT MarshalIDataObjectInStream( IDataObject* pIDataObject, LPSTREAM* ppStream )
{
    OSL_ASSERT( nullptr != pIDataObject );
    assert(nullptr != ppStream);

    *ppStream = nullptr;
    return CoMarshalInterThreadInterfaceInStream(
        __uuidof(IDataObject),  //The IID of interface to be marshalled
        pIDataObject,           //The interface pointer
        ppStream                //IStream pointer
        );
}

// unmarshal an IDataObject
HRESULT UnmarshalIDataObjectAndReleaseStream( LPSTREAM lpStream, IDataObject** ppIDataObject )
{
    OSL_ASSERT( nullptr != lpStream );
    assert(nullptr != ppIDataObject);

    *ppIDataObject = nullptr;
    return CoGetInterfaceAndReleaseStream(
        lpStream,
        __uuidof(IDataObject),
        reinterpret_cast<LPVOID*>(ppIDataObject));
}

// message handler functions; remember these functions are called from a different thread context!

void onSetClipboard(IDataObject* pIDataObject)
{
    sal::systools::RetryIfFailed(10, 100, [pIDataObject] { return OleSetClipboard(pIDataObject); });
}

HRESULT onGetClipboard(LPSTREAM* ppStream)
{
    OSL_ASSERT(nullptr != ppStream);

    IDataObjectPtr pIDataObject;

    // forward the request to the OleClipboard
    HRESULT hr
        = sal::systools::RetryIfFailed(10, 100, [p = &pIDataObject] { return OleGetClipboard(p); });
    if (SUCCEEDED(hr))
    {
        hr = MarshalIDataObjectInStream(pIDataObject.get(), ppStream);
        OSL_ENSURE(SUCCEEDED(hr), "marshalling clipboard data object failed");
    }
    return hr;
}

// flush the ole-clipboard
HRESULT onFlushClipboard()
{
    return sal::systools::RetryIfFailed(10, 100, [] { return OleFlushClipboard(); });
}

// Use a template to localize reinterpret_cast
template <typename wParam_t, typename lParam_t>
bool postMessage(HWND hWnd, UINT msg, wParam_t wParam, lParam_t lParam)
{
    bool const ret = PostMessageW(hWnd, msg, reinterpret_cast<WPARAM>(wParam),
                                  reinterpret_cast<LPARAM>(lParam));
    SAL_WARN_IF(!ret, "vcl.win.dtrans", "ERROR: PostMessage() failed!");
    return ret;
}

// helper class to ensure that the calling thread has com initialized
class CAutoComInit
{
public:
   /*
       to be safe we call CoInitializeEx
       although it is not necessary if
       the calling thread was created
       using osl_CreateThread because
       this function calls CoInitializeEx
       for every thread it creates
    */
    CAutoComInit( ) : m_hResult( CoInitializeEx( nullptr, COINIT_APARTMENTTHREADED ) )
    {
        if ( S_OK == m_hResult )
            OSL_FAIL(
            "com was not yet initialized, the thread was not created using osl_createThread" );
        else if ( FAILED( m_hResult ) && !( RPC_E_CHANGED_MODE == m_hResult ) )
            OSL_FAIL(
            "com could not be initialized, maybe the thread was not created using osl_createThread" );
    }

    ~CAutoComInit( )
    {
        /*
            we only call CoUninitialize when
            CoInitializeEx returned S_FALSE, what
            means that com was already initialize
            for that thread so we keep the balance
            if CoInitializeEx returned S_OK what means
            com was not yet initialized we better
            let com initialized or we may run into
            the realm of undefined behaviour
        */
        if ( m_hResult == S_FALSE )
            CoUninitialize( );
    }

private:
    HRESULT m_hResult;
};

}

CMtaOleClipboard::CMtaOleClipboard( ) :
    m_hOleThread( nullptr ),
    m_uOleThreadId( 0 ),
    // signals that the thread was successfully setup
    m_hEvtThrdReady(CreateEventW( nullptr, MANUAL_RESET, INIT_NONSIGNALED, nullptr )),
    m_hwndMtaOleReqWnd( nullptr ),
    // signals that the window is destroyed - to stop waiting any winproc result
    m_hEvtWndDisposed(CreateEventW(nullptr, MANUAL_RESET, INIT_NONSIGNALED, nullptr)),
    m_MtaOleReqWndClassAtom( 0 ),
    m_pfncClipViewerCallback( nullptr ),
    m_bRunClipboardNotifierThread( true ),
    m_hClipboardChangedEvent( m_hClipboardChangedNotifierEvents[0] ),
    m_hTerminateClipboardChangedNotifierEvent( m_hClipboardChangedNotifierEvents[1] ),
    m_ClipboardChangedEventCount( 0 )
{
    OSL_ASSERT( nullptr != m_hEvtThrdReady );
    SAL_WARN_IF(!m_hEvtWndDisposed, "vcl.win.dtrans", "CreateEventW failed: m_hEvtWndDisposed is nullptr");

    s_theMtaOleClipboardInst = this;

    m_hOleThread = reinterpret_cast<HANDLE>(_beginthreadex(
        nullptr, 0, CMtaOleClipboard::oleThreadProc, this, 0, &m_uOleThreadId ));
    OSL_ASSERT( nullptr != m_hOleThread );

    // setup the clipboard changed notifier thread

    m_hClipboardChangedNotifierEvents[0] = CreateEventW( nullptr, MANUAL_RESET, INIT_NONSIGNALED, nullptr );
    OSL_ASSERT( nullptr != m_hClipboardChangedNotifierEvents[0] );

    m_hClipboardChangedNotifierEvents[1] = CreateEventW( nullptr, MANUAL_RESET, INIT_NONSIGNALED, nullptr );
    OSL_ASSERT( nullptr != m_hClipboardChangedNotifierEvents[1] );

    m_hClipboardChangedNotifierThread = reinterpret_cast<HANDLE>(_beginthreadex(
        nullptr, 0, CMtaOleClipboard::clipboardChangedNotifierThreadProc, this, 0, nullptr ));

    OSL_ASSERT( nullptr != m_hClipboardChangedNotifierThread );
}

CMtaOleClipboard::~CMtaOleClipboard( )
{
    // block calling threads out
    if ( nullptr != m_hEvtThrdReady )
        ResetEvent( m_hEvtThrdReady );

    // terminate the clipboard changed notifier thread
    m_bRunClipboardNotifierThread = false;
    SetEvent( m_hTerminateClipboardChangedNotifierEvent );

    // unblock whoever could still wait for event processing
    if (m_hEvtWndDisposed)
        SetEvent(m_hEvtWndDisposed);

    sal_uInt32 dwResult = WaitForSingleObject(
        m_hClipboardChangedNotifierThread, MAX_WAIT_SHUTDOWN );

    OSL_ENSURE( dwResult == WAIT_OBJECT_0, "clipboard notifier thread could not terminate" );

    if ( nullptr != m_hClipboardChangedNotifierThread )
        CloseHandle( m_hClipboardChangedNotifierThread );

    if ( nullptr != m_hClipboardChangedNotifierEvents[0] )
        CloseHandle( m_hClipboardChangedNotifierEvents[0] );

    if ( nullptr != m_hClipboardChangedNotifierEvents[1] )
        CloseHandle( m_hClipboardChangedNotifierEvents[1] );

    // end the thread
    // because DestroyWindow can only be called
    // from within the thread that created the window
    SendMessageW(m_hwndMtaOleReqWnd, MSG_SHUTDOWN, 0, 0);

    // wait for thread shutdown
    dwResult = WaitForSingleObject( m_hOleThread, MAX_WAIT_SHUTDOWN );
    OSL_ENSURE( dwResult == WAIT_OBJECT_0, "OleThread could not terminate" );

    if ( nullptr != m_hOleThread )
        CloseHandle( m_hOleThread );

    if ( nullptr != m_hEvtThrdReady )
        CloseHandle( m_hEvtThrdReady );

    if (m_hEvtWndDisposed)
        CloseHandle(m_hEvtWndDisposed);

    if ( m_MtaOleReqWndClassAtom )
        UnregisterClassW( g_szWndClsName, nullptr );

    OSL_ENSURE( ( nullptr == m_pfncClipViewerCallback ),
                "Clipboard viewer not properly unregistered" );
}

void CMtaOleClipboard::flushClipboard()
{
    if ( !WaitForThreadReady( ) )
    {
        OSL_FAIL( "clipboard sta thread not ready" );
        return;
    }

    OSL_ENSURE( GetCurrentThreadId( ) != m_uOleThreadId,
        "flushClipboard from within clipboard sta thread called" );

    MsgCtx  aMsgCtx;

    postMessage(m_hwndMtaOleReqWnd, MSG_FLUSHCLIPBOARD, nullptr, &aMsgCtx);
    aMsgCtx.aCondition.wait(m_hEvtWndDisposed);
}

HRESULT CMtaOleClipboard::getClipboard( IDataObject** ppIDataObject )
{
    assert(ppIDataObject && "invalid parameter");
    OSL_PRECOND( GetCurrentThreadId( ) != m_uOleThreadId, "getClipboard from within clipboard sta thread called" );

    if ( !WaitForThreadReady( ) )
    {
        OSL_FAIL( "clipboard sta thread not ready" );
        return E_FAIL;
    }

    CAutoComInit comAutoInit;

    LPSTREAM lpStream;

    *ppIDataObject = nullptr;

    MsgCtx    aMsgCtx;

    const bool bWaitSuccess = postMessage(m_hwndMtaOleReqWnd, MSG_GETCLIPBOARD, &lpStream, &aMsgCtx)
                              && aMsgCtx.aCondition.wait(m_hEvtWndDisposed);

    HRESULT hr = bWaitSuccess ? aMsgCtx.hr : E_ABORT;

    if ( SUCCEEDED( hr ) )
    {
        hr = UnmarshalIDataObjectAndReleaseStream( lpStream, ppIDataObject );
        OSL_ENSURE( SUCCEEDED( hr ), "unmarshalling clipboard data object failed" );
    }

    return hr;
}

// this is an asynchronous method that's why we don't wait until the
// request is completed
void CMtaOleClipboard::setClipboard(IDataObject* pIDataObject)
{
    if ( !WaitForThreadReady( ) )
    {
        OSL_FAIL( "clipboard sta thread not ready" );
        return;
    }

    CAutoComInit comAutoInit;

    OSL_ENSURE( GetCurrentThreadId( ) != m_uOleThreadId, "setClipboard from within the clipboard sta thread called" );

    // because we marshall this request
    // into the sta thread we better
    // acquire the interface here so
    // that the object will not be
    // destroyed before the ole clipboard
    // can acquire it
    // remember: pIDataObject may be NULL
    // which is a request to clear the
    // current clipboard content
    if ( pIDataObject )
        pIDataObject->AddRef( );

    postMessage(m_hwndMtaOleReqWnd, MSG_SETCLIPBOARD, pIDataObject, nullptr);
}

// register a clipboard viewer
void CMtaOleClipboard::registerClipViewer(LPFNC_CLIPVIEWER_CALLBACK_t pfncClipViewerCallback)
{
    if ( !WaitForThreadReady( ) )
    {
        OSL_FAIL( "clipboard sta thread not ready" );
        return;
    }

    OSL_ENSURE( GetCurrentThreadId( ) != m_uOleThreadId, "registerClipViewer from within the OleThread called" );

    MsgCtx aMsgCtx;

    if (postMessage(m_hwndMtaOleReqWnd, MSG_REGCLIPVIEWER, pfncClipViewerCallback, &aMsgCtx))
        aMsgCtx.aCondition.wait(m_hEvtWndDisposed);
}

// register a clipboard viewer
void CMtaOleClipboard::onRegisterClipViewer(LPFNC_CLIPVIEWER_CALLBACK_t pfncClipViewerCallback)
{
    // we need exclusive access because the clipboard changed notifier
    // thread also accesses this variable
    std::unique_lock aGuard( m_pfncClipViewerCallbackMutex );

    // register if not yet done
    if ( ( nullptr != pfncClipViewerCallback ) && ( nullptr == m_pfncClipViewerCallback ) )
    {
        // SetClipboardViewer sends a WM_DRAWCLIPBOARD message we ignore
        // this message if we register ourself as clip viewer
        m_bInRegisterClipViewer = true;
        AddClipboardFormatListener(m_hwndMtaOleReqWnd);
        m_bInRegisterClipViewer = false;

        // save the new callback function
        m_pfncClipViewerCallback = pfncClipViewerCallback;
    }
    else if ( ( nullptr == pfncClipViewerCallback ) && ( nullptr != m_pfncClipViewerCallback ) )
    {
        m_pfncClipViewerCallback = nullptr;

        // unregister if input parameter is NULL and we previously registered
        // as clipboard viewer
        RemoveClipboardFormatListener(m_hwndMtaOleReqWnd);
    }
}

// handle clipboard update event
LRESULT CMtaOleClipboard::onClipboardUpdate()
{
    // we don't send a notification if we are
    // registering ourself as clipboard
    if ( !m_bInRegisterClipViewer )
    {
        std::unique_lock aGuard( m_ClipboardChangedEventCountMutex );

        m_ClipboardChangedEventCount++;
        SetEvent( m_hClipboardChangedEvent );
    }

    return 0;
}

// the window proc
LRESULT CALLBACK CMtaOleClipboard::mtaOleReqWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LRESULT lResult = 0;

    // get a connection to the class-instance via the static
    CMtaOleClipboard* pImpl = s_theMtaOleClipboardInst;
    assert(pImpl);

    switch( uMsg )
    {
    case MSG_SETCLIPBOARD:
        {
            IDataObject* pIDataObject = reinterpret_cast< IDataObject* >( wParam );
            onSetClipboard(pIDataObject);

            // in setClipboard we did acquire the
            // interface pointer in order to prevent
            // destruction of the object before the
            // ole clipboard can acquire the interface
            // now we release the interface so that
            // our lostOwnership mechanism works
            // remember: pIDataObject may be NULL
            if ( pIDataObject )
                pIDataObject->Release( );
        }
        break;

    case MSG_GETCLIPBOARD:
        {
            MsgCtx* aMsgCtx = reinterpret_cast< MsgCtx* >( lParam );
            assert(aMsgCtx);

            aMsgCtx->hr = onGetClipboard(reinterpret_cast<LPSTREAM*>(wParam));
            aMsgCtx->aCondition.set( );
        }
        break;

    case MSG_FLUSHCLIPBOARD:
        {
            MsgCtx* aMsgCtx = reinterpret_cast< MsgCtx* >( lParam );
            assert(aMsgCtx);

            aMsgCtx->hr = onFlushClipboard();
            aMsgCtx->aCondition.set( );
        }
        break;

    case MSG_REGCLIPVIEWER:
        {
            MsgCtx* pMsgCtx = reinterpret_cast<MsgCtx*>(lParam);
            assert(pMsgCtx && "pMsgCtx is nullptr");

            pImpl->onRegisterClipViewer(
                reinterpret_cast<CMtaOleClipboard::LPFNC_CLIPVIEWER_CALLBACK_t>(wParam));
            pMsgCtx->aCondition.set();
        }
        break;

    case WM_CLIPBOARDUPDATE:
        lResult = pImpl->onClipboardUpdate();
        break;

    case MSG_SHUTDOWN:
        DestroyWindow( pImpl->m_hwndMtaOleReqWnd );
        break;

    // force the sta thread to end
    case WM_DESTROY:
        SetEvent(pImpl->m_hEvtWndDisposed); // stop waiting for conditions set by this wndproc
        PostQuitMessage( 0 );
        break;

    default:
        lResult = DefWindowProcW( hWnd, uMsg, wParam, lParam );
        break;
    }

    return lResult;
}

void CMtaOleClipboard::createMtaOleReqWnd( )
{
    WNDCLASSEXW  wcex;

    SalData* pSalData = GetSalData();
    OSL_ASSERT(nullptr != pSalData->mhInst);

    ZeroMemory( &wcex, sizeof(wcex) );

    wcex.cbSize         = sizeof(wcex);
    wcex.style          = 0;
    wcex.lpfnWndProc    = CMtaOleClipboard::mtaOleReqWndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = pSalData->mhInst;
    wcex.hIcon          = nullptr;
    wcex.hCursor        = nullptr;
    wcex.hbrBackground  = nullptr;
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = g_szWndClsName;
    wcex.hIconSm        = nullptr;

    m_MtaOleReqWndClassAtom = RegisterClassExW( &wcex );

    if ( 0 != m_MtaOleReqWndClassAtom )
        m_hwndMtaOleReqWnd = CreateWindowW(
            g_szWndClsName, nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, pSalData->mhInst, nullptr );
}

unsigned int CMtaOleClipboard::run( )
{
    HRESULT hr = OleInitialize( nullptr );
    OSL_ASSERT( SUCCEEDED( hr ) );

    createMtaOleReqWnd( );

    unsigned int nRet = ~0U; // = error

    if ( IsWindow( m_hwndMtaOleReqWnd ) )
    {
        if ( nullptr != m_hEvtThrdReady )
            SetEvent( m_hEvtThrdReady );

        nRet = 0;

        // pumping messages
        for (;;)
        {
            MSG msg;
            int const bRet = GetMessageW(&msg, nullptr, 0, 0);
            if (bRet == 0)
            {
                break;
            }
            if (-1 == bRet)
            {
                SAL_WARN("vcl.win.dtrans", "GetMessageW failed: " << comphelper::WindowsErrorString(GetLastError()));
                nRet = ~0U;
                break;
            }
            DispatchMessageW(&msg);
        }
    }

    OleUninitialize( );

    return nRet;
}

unsigned __stdcall CMtaOleClipboard::oleThreadProc( void* pParam )
{
    osl_setThreadName("CMtaOleClipboard::run()");

    CMtaOleClipboard* pInst =
        static_cast<CMtaOleClipboard*>( pParam );
    OSL_ASSERT( nullptr != pInst );

    return pInst->run( );
}

unsigned __stdcall CMtaOleClipboard::clipboardChangedNotifierThreadProc(void* pParam)
{
    osl_setThreadName("CMtaOleClipboard::clipboardChangedNotifierThreadProc()");
    CMtaOleClipboard* pInst = static_cast< CMtaOleClipboard* >( pParam );
    assert(pInst);

    sal::systools::CoInitializeGuard aGuard(COINIT_APARTMENTTHREADED, false,
                                            sal::systools::CoInitializeGuard::WhenFailed::NoThrow);

    // assuming we don't need a lock for
    // a boolean variable like m_bRun...
    while ( pInst->m_bRunClipboardNotifierThread )
    {
        // process window messages because of CoInitializeEx
        MSG Msg;
        while (PeekMessageW(&Msg, nullptr, 0, 0, PM_REMOVE))
            DispatchMessageW(&Msg);

        // wait for clipboard changed or terminate event
        MsgWaitForMultipleObjects(2, pInst->m_hClipboardChangedNotifierEvents, false, INFINITE,
                                  QS_ALLINPUT | QS_ALLPOSTMESSAGE);

        bool hadEvents;
        {
            std::unique_lock aGuard2(pInst->m_ClipboardChangedEventCountMutex);
            hadEvents = pInst->m_ClipboardChangedEventCount > 0;
            if (hadEvents)
            {
                pInst->m_ClipboardChangedEventCount--;
                if (0 == pInst->m_ClipboardChangedEventCount)
                    ResetEvent(pInst->m_hClipboardChangedEvent);
            }
        }

        if (hadEvents)
        {
            LPFNC_CLIPVIEWER_CALLBACK_t pClipViewerCallback;
            {
                // nobody should touch m_pfncClipViewerCallback while we do
                // but don't hold the mutex while calling the callback itself: it could deadlock
                std::unique_lock aClipViewerGuard(pInst->m_pfncClipViewerCallbackMutex);
                pClipViewerCallback = pInst->m_pfncClipViewerCallback;
            }

            // notify all clipboard listener
            if (pClipViewerCallback)
                pClipViewerCallback();
        }
    }

    return 0;
}

bool CMtaOleClipboard::WaitForThreadReady( ) const
{
    bool bRet = false;

    if ( nullptr != m_hEvtThrdReady )
    {
        DWORD dwResult = WaitForSingleObject(
            m_hEvtThrdReady, MAX_WAITTIME );
        bRet = ( dwResult == WAIT_OBJECT_0 );
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
