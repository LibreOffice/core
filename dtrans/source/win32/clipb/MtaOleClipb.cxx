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
    hidden window an forward these requests via window messages.
*/

#ifdef _MSC_VER
#pragma warning( disable : 4786 ) // identifier was truncated to 'number'
                                   // characters in the debug information
#endif

//#define UNICODE
#include <osl/diagnose.h>

#include "MtaOleClipb.hxx"
#include <osl/conditn.hxx>

#include <wchar.h>
#include <process.h>

#include <systools/win32/comtools.hxx>
#ifdef __MINGW32__
#if defined __uuidof
#undef __uuidof
#endif
#define __uuidof(I) IID_##I
#endif

//----------------------------------------------------------------
//  namespace directives
//----------------------------------------------------------------

using osl::Condition;
using osl::Mutex;
using osl::MutexGuard;
using osl::ClearableMutexGuard;

namespace /* private */
{
    char CLIPSRV_DLL_NAME[] = "sysdtrans.dll";
    char g_szWndClsName[]   = "MtaOleReqWnd###";

    //--------------------------------------------------------
    // messages constants
    //--------------------------------------------------------

    const sal_uInt32 MSG_SETCLIPBOARD               = WM_USER + 0x0001;
    const sal_uInt32 MSG_GETCLIPBOARD               = WM_USER + 0x0002;
    const sal_uInt32 MSG_REGCLIPVIEWER              = WM_USER + 0x0003;
    const sal_uInt32 MSG_FLUSHCLIPBOARD             = WM_USER + 0x0004;
    const sal_uInt32 MSG_SHUTDOWN                   = WM_USER + 0x0005;

    const sal_uInt32 MAX_WAITTIME                   = 10000;  // msec
    const sal_uInt32 MAX_WAIT_SHUTDOWN              = 10000; // msec
    const sal_uInt32 MAX_CLIPEVENT_PROCESSING_TIME  = 5000;  // msec

    const sal_Bool MANUAL_RESET                     = sal_True;
    const sal_Bool AUTO_RESET                       = sal_False;
    const sal_Bool INIT_NONSIGNALED                 = sal_False;

    //------------------------------------------------------
    /*  Cannot use osl conditions because they are blocking
        without waking up on messages sent by another thread
        this leads to deadlocks because we are blocking the
        communication between inter-thread marshalled COM
        pointers.
        COM Proxy-Stub communication uses SendMessages for
        synchronization purposes.
    */
    class Win32Condition
    {
        public:
            // ctor
            Win32Condition()
            {
                m_hEvent = CreateEvent(
                    0,      /* no security */
                    true,   /* manual reset */
                    false,  /* initial state not signaled */
                    0);     /* automatic name */
            }

            // dtor
            ~Win32Condition()
            {
                CloseHandle(m_hEvent);
            }

            // wait infinite for event be signaled
            // leave messages sent through
            void wait()
            {
                while(1)
                {
                    DWORD dwResult =
                        MsgWaitForMultipleObjects(1, &m_hEvent, FALSE, INFINITE, QS_SENDMESSAGE);

                       switch (dwResult)
                    {
                        case WAIT_OBJECT_0:
                            return;

                        case WAIT_OBJECT_0 + 1:
                        {
                            /* PeekMessage processes all messages in the SendMessage
                               queue that's what we want, messages from the PostMessage
                               queue stay untouched */
                            MSG msg;
                               PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

                            break;
                        }
                    }
                }
            }

            // reset the event
            void set()
            {
                SetEvent(m_hEvent);
            }

        private:
            HANDLE m_hEvent;

        // prevent copy/assignment
        private:
            Win32Condition(const Win32Condition&);
            Win32Condition& operator=(const Win32Condition&);
    };

    //------------------------------------------
    // we use one condition for every request
    //------------------------------------------

    struct MsgCtx
    {
        Win32Condition  aCondition;
        HRESULT         hr;
    };

} /* namespace private */

//----------------------------------------------------------------
//  static member initialization
//----------------------------------------------------------------

CMtaOleClipboard* CMtaOleClipboard::s_theMtaOleClipboardInst = NULL;

//--------------------------------------------------------------------
// marshal an IDataObject
//--------------------------------------------------------------------

//inline
HRESULT MarshalIDataObjectInStream( IDataObject* pIDataObject, LPSTREAM* ppStream )
{
    OSL_ASSERT( NULL != pIDataObject );
    OSL_ASSERT( NULL != ppStream );

    *ppStream = NULL;
    return CoMarshalInterThreadInterfaceInStream(
        __uuidof(IDataObject),  //The IID of inteface to be marshaled
        pIDataObject,           //The interface pointer
        ppStream                //IStream pointer
        );
}

//--------------------------------------------------------------------
// unmarshal an IDataObject
//--------------------------------------------------------------------

//inline
HRESULT UnmarshalIDataObjectAndReleaseStream( LPSTREAM lpStream, IDataObject** ppIDataObject )
{
    OSL_ASSERT( NULL != lpStream );
    OSL_ASSERT( NULL != ppIDataObject );

    *ppIDataObject = NULL;
    return CoGetInterfaceAndReleaseStream(
        lpStream,
        __uuidof(IDataObject),
        reinterpret_cast<LPVOID*>(ppIDataObject));
}

//--------------------------------------------------------------------
// helper class to ensure that the calling thread has com initialized
//--------------------------------------------------------------------

class CAutoComInit
{
public:
    CAutoComInit( )
    {
        /*
            to be safe we call CoInitialize
            although it is not necessary if
            the calling thread was created
            using osl_CreateThread because
            this function calls CoInitialize
            for every thread it creates
        */
        m_hResult = CoInitialize( NULL );

        if ( S_OK == m_hResult )
            OSL_FAIL( \
            "com was not yet initialzed, the thread was not created using osl_createThread" );
        else if ( FAILED( m_hResult ) && !( RPC_E_CHANGED_MODE == m_hResult ) )
            OSL_FAIL( \
            "com could not be initialized, maybe the thread was not created using osl_createThread" );
    }

    ~CAutoComInit( )
    {
        /*
            we only call CoUninitialize when
            CoInitailize returned S_FALSE, what
            means that com was already initialize
            for that thread so we keep the balance
            if CoInitialize returned S_OK what means
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

//--------------------------------------------------------------------
// ctor
//--------------------------------------------------------------------

CMtaOleClipboard::CMtaOleClipboard( ) :
    m_hOleThread( NULL ),
    m_uOleThreadId( 0 ),
    m_hEvtThrdReady( NULL ),
    m_hwndMtaOleReqWnd( NULL ),
    m_MtaOleReqWndClassAtom( 0 ),
    m_hwndNextClipViewer( NULL ),
    m_pfncClipViewerCallback( NULL ),
    m_bRunClipboardNotifierThread( true ),
    m_hClipboardChangedEvent( m_hClipboardChangedNotifierEvents[0] ),
    m_hTerminateClipboardChangedNotifierEvent( m_hClipboardChangedNotifierEvents[1] ),
    m_ClipboardChangedEventCount( 0 )
{
    // signals that the thread was successfully setup
    m_hEvtThrdReady  = CreateEventA( 0, MANUAL_RESET, INIT_NONSIGNALED, NULL );

    OSL_ASSERT( NULL != m_hEvtThrdReady );

    s_theMtaOleClipboardInst = this;

    m_hOleThread = (HANDLE)_beginthreadex(
        NULL, 0, CMtaOleClipboard::oleThreadProc, this, 0, &m_uOleThreadId );
    OSL_ASSERT( NULL != m_hOleThread );

    //----------------------------------------------
    // setup the clipboard changed notifier thread
    //----------------------------------------------

    m_hClipboardChangedNotifierEvents[0] = CreateEventA( 0, MANUAL_RESET, INIT_NONSIGNALED, NULL );
    OSL_ASSERT( NULL != m_hClipboardChangedNotifierEvents[0] );

    m_hClipboardChangedNotifierEvents[1] = CreateEventA( 0, MANUAL_RESET, INIT_NONSIGNALED, NULL );
    OSL_ASSERT( NULL != m_hClipboardChangedNotifierEvents[1] );

    unsigned uThreadId;
    m_hClipboardChangedNotifierThread = (HANDLE)_beginthreadex(
        NULL, 0, CMtaOleClipboard::clipboardChangedNotifierThreadProc, this, 0, &uThreadId );

    OSL_ASSERT( NULL != m_hClipboardChangedNotifierThread );
}

//--------------------------------------------------------------------
// dtor
//--------------------------------------------------------------------

CMtaOleClipboard::~CMtaOleClipboard( )
{
    // block calling threads out
    if ( NULL != m_hEvtThrdReady )
        ResetEvent( m_hEvtThrdReady );

    // terminate the clipboard changed notifier thread
    m_bRunClipboardNotifierThread = false;
    SetEvent( m_hTerminateClipboardChangedNotifierEvent );

    sal_uInt32 dwResult = WaitForSingleObject(
        m_hClipboardChangedNotifierThread, MAX_WAIT_SHUTDOWN );

    (void) dwResult;
    OSL_ENSURE( dwResult == WAIT_OBJECT_0, "clipboard notifier thread could not terminate" );

    if ( NULL != m_hClipboardChangedNotifierThread )
        CloseHandle( m_hClipboardChangedNotifierThread );

    if ( NULL != m_hClipboardChangedNotifierEvents[0] )
        CloseHandle( m_hClipboardChangedNotifierEvents[0] );

    if ( NULL != m_hClipboardChangedNotifierEvents[1] )
        CloseHandle( m_hClipboardChangedNotifierEvents[1] );

    // end the thread
    // because DestroyWindow can only be called
    // from within the thread that created the window
    sendMessage( MSG_SHUTDOWN,
                 static_cast< WPARAM >( 0 ),
                 static_cast< LPARAM >( 0 ) );

    // wait for thread shutdown
    dwResult = WaitForSingleObject( m_hOleThread, MAX_WAIT_SHUTDOWN );
    OSL_ENSURE( dwResult == WAIT_OBJECT_0, "OleThread could not terminate" );

    if ( NULL != m_hOleThread )
        CloseHandle( m_hOleThread );

    if ( NULL != m_hEvtThrdReady )
        CloseHandle( m_hEvtThrdReady );

    if ( m_MtaOleReqWndClassAtom )
        UnregisterClassA( g_szWndClsName, NULL );

    OSL_ENSURE( ( NULL == m_pfncClipViewerCallback ) &&
                !IsWindow( m_hwndNextClipViewer ), \
                "Clipboard viewer not properly unregistered" );
}


//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

HRESULT CMtaOleClipboard::flushClipboard( )
{
    if ( !WaitForThreadReady( ) )
    {
        OSL_FAIL( "clipboard sta thread not ready" );
        return E_FAIL;
    }

    OSL_ENSURE( GetCurrentThreadId( ) != m_uOleThreadId, \
        "flushClipboard from within clipboard sta thread called" );

    MsgCtx  aMsgCtx;

    postMessage( MSG_FLUSHCLIPBOARD,
                 static_cast< WPARAM >( 0 ),
                 reinterpret_cast< LPARAM >( &aMsgCtx ) );

    aMsgCtx.aCondition.wait( /* infinite */ );

    return aMsgCtx.hr;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

HRESULT CMtaOleClipboard::getClipboard( IDataObject** ppIDataObject )
{
    OSL_PRECOND( NULL != ppIDataObject, "invalid parameter" );
    OSL_PRECOND( GetCurrentThreadId( ) != m_uOleThreadId, "getClipboard from within clipboard sta thread called" );

    if ( !WaitForThreadReady( ) )
    {
        OSL_FAIL( "clipboard sta thread not ready" );
        return E_FAIL;
    }

    CAutoComInit comAutoInit;

    LPSTREAM lpStream;
    HRESULT  hr = E_FAIL;

    *ppIDataObject = NULL;

    MsgCtx    aMsgCtx;

    postMessage( MSG_GETCLIPBOARD,
                 reinterpret_cast< WPARAM >( &lpStream ),
                 reinterpret_cast< LPARAM >( &aMsgCtx ) );

    aMsgCtx.aCondition.wait( /* infinite */ );

    hr = aMsgCtx.hr;

    if ( SUCCEEDED( hr ) )
    {
        hr = UnmarshalIDataObjectAndReleaseStream( lpStream, ppIDataObject );
        OSL_ENSURE( SUCCEEDED( hr ), "unmarshalling clipboard data object failed" );
    }

    return hr;
}

//--------------------------------------------------------------------
// this is an asynchronous method that's why we don't wait until the
// request is completed
//--------------------------------------------------------------------

HRESULT CMtaOleClipboard::setClipboard( IDataObject* pIDataObject )
{
    if ( !WaitForThreadReady( ) )
    {
        OSL_FAIL( "clipboard sta thread not ready" );
        return E_FAIL;
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
    // which is an request to clear the
    // current clipboard content
    if ( pIDataObject )
        pIDataObject->AddRef( );

    postMessage(
        MSG_SETCLIPBOARD,
        reinterpret_cast< WPARAM >( pIDataObject ),
        0 );

    // because this is an asynchronous function
    // the return value is useless
    return S_OK;
}

//--------------------------------------------------------------------
// register a clipboard viewer
//--------------------------------------------------------------------

bool CMtaOleClipboard::registerClipViewer( LPFNC_CLIPVIEWER_CALLBACK_t pfncClipViewerCallback )
{
    if ( !WaitForThreadReady( ) )
    {
        OSL_FAIL( "clipboard sta thread not ready" );
        return false;
    }

    bool bRet = false;

    OSL_ENSURE( GetCurrentThreadId( ) != m_uOleThreadId, "registerClipViewer from within the OleThread called" );

    MsgCtx  aMsgCtx;

    postMessage( MSG_REGCLIPVIEWER,
                 reinterpret_cast<WPARAM>( pfncClipViewerCallback ),
                 reinterpret_cast<LPARAM>( &aMsgCtx ) );

    aMsgCtx.aCondition.wait( /* infinite */ );

    return bRet;
}

//--------------------------------------------------------------------
// register a clipboard viewer
//--------------------------------------------------------------------

bool CMtaOleClipboard::onRegisterClipViewer( LPFNC_CLIPVIEWER_CALLBACK_t pfncClipViewerCallback )
{
    bool bRet = true;

    // we need exclusive access because the clipboard changed notifier
    // thread also accesses this variable
    MutexGuard aGuard( m_pfncClipViewerCallbackMutex );

    // register if not yet done
    if ( ( NULL != pfncClipViewerCallback ) && ( NULL == m_pfncClipViewerCallback ) )
    {
        // SetClipboardViewer sends a WM_DRAWCLIPBOARD message we ignore
        // this message if we register ourself as clip viewer
        m_bInRegisterClipViewer = true;
        m_hwndNextClipViewer = SetClipboardViewer( m_hwndMtaOleReqWnd );
        m_bInRegisterClipViewer = false;

        // if there is no other cb-viewer the
        // return value is NULL!!!
        bRet = IsWindow( m_hwndNextClipViewer ) ? true : false;

        // save the new callback function
        m_pfncClipViewerCallback = pfncClipViewerCallback;
    }
    else if ( ( NULL == pfncClipViewerCallback ) && ( NULL != m_pfncClipViewerCallback ) )
    {
        m_pfncClipViewerCallback = NULL;

        // unregister if input parameter is NULL and we previously registered
        // as clipboard viewer
        ChangeClipboardChain( m_hwndMtaOleReqWnd, m_hwndNextClipViewer );
        m_hwndNextClipViewer = NULL;
    }

    return bRet;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

LRESULT CMtaOleClipboard::onSetClipboard( IDataObject* pIDataObject )
{
    return static_cast<LRESULT>( OleSetClipboard( pIDataObject ) );
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

LRESULT CMtaOleClipboard::onGetClipboard( LPSTREAM* ppStream )
{
    OSL_ASSERT(NULL != ppStream);

    IDataObjectPtr pIDataObject;

    // forward the request to the OleClipboard
    HRESULT hr = OleGetClipboard( &pIDataObject );
    if ( SUCCEEDED( hr ) )
    {
        hr = MarshalIDataObjectInStream(pIDataObject.get(), ppStream);
        OSL_ENSURE(SUCCEEDED(hr), "marshalling cliboard data object failed");
    }
    return static_cast<LRESULT>(hr);
}

//--------------------------------------------------------------------
// flush the ole-clipboard
//--------------------------------------------------------------------

LRESULT CMtaOleClipboard::onFlushClipboard( )
{
    return static_cast<LRESULT>( OleFlushClipboard( ) );
}

//--------------------------------------------------------------------
// handle clipboard chain change event
//--------------------------------------------------------------------

LRESULT CMtaOleClipboard::onChangeCBChain( HWND hWndRemove, HWND hWndNext )
{
    if ( hWndRemove == m_hwndNextClipViewer )
        m_hwndNextClipViewer = hWndNext;
    else if ( IsWindow( m_hwndNextClipViewer ) )
    {
        // forward the message to the next one
        DWORD_PTR dwpResult;
        SendMessageTimeoutA(
            m_hwndNextClipViewer,
            WM_CHANGECBCHAIN,
            reinterpret_cast<WPARAM>(hWndRemove),
            reinterpret_cast<LPARAM>(hWndNext),
            SMTO_BLOCK,
            MAX_CLIPEVENT_PROCESSING_TIME,
            &dwpResult );
    }

    return 0;
}

//--------------------------------------------------------------------
// handle draw clipboard event
//--------------------------------------------------------------------

LRESULT CMtaOleClipboard::onDrawClipboard( )
{
    // we don't send a notification if we are
    // registering ourself as clipboard
    if ( !m_bInRegisterClipViewer )
    {
        ClearableMutexGuard aGuard( m_ClipboardChangedEventCountMutex );

        m_ClipboardChangedEventCount++;
        SetEvent( m_hClipboardChangedEvent );

        aGuard.clear( );
    }

    // foward the message to the next viewer in the chain
    if ( IsWindow( m_hwndNextClipViewer ) )
    {
        DWORD_PTR dwpResult;
        SendMessageTimeoutA(
            m_hwndNextClipViewer,
            WM_DRAWCLIPBOARD,
            static_cast< WPARAM >( 0 ),
            static_cast< LPARAM >( 0 ),
            SMTO_BLOCK,
            MAX_CLIPEVENT_PROCESSING_TIME,
            &dwpResult );
    }

    return 0;
}

//--------------------------------------------------------------------
// SendMessage so we don't need to supply the HWND if we send
// something to our wrapped window
//--------------------------------------------------------------------

LRESULT CMtaOleClipboard::sendMessage( UINT msg, WPARAM wParam, LPARAM lParam )
{
    return ::SendMessageA( m_hwndMtaOleReqWnd, msg, wParam, lParam );
}

//--------------------------------------------------------------------
// PostMessage so we don't need to supply the HWND if we send
// something to our wrapped window
//--------------------------------------------------------------------

bool CMtaOleClipboard::postMessage( UINT msg, WPARAM wParam, LPARAM lParam )
{
    return PostMessageA( m_hwndMtaOleReqWnd, msg, wParam, lParam ) ? true : false;
}


//--------------------------------------------------------------------
// the window proc
//--------------------------------------------------------------------

LRESULT CALLBACK CMtaOleClipboard::mtaOleReqWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LRESULT lResult = 0;

    // get a connection to the class-instance via the static member
    CMtaOleClipboard* pImpl = CMtaOleClipboard::s_theMtaOleClipboardInst;
    OSL_ASSERT( NULL != pImpl );

    switch( uMsg )
    {
    case MSG_SETCLIPBOARD:
        {
            IDataObject* pIDataObject = reinterpret_cast< IDataObject* >( wParam );
            pImpl->onSetClipboard( pIDataObject );

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
            OSL_ASSERT( aMsgCtx );

            aMsgCtx->hr = pImpl->onGetClipboard( reinterpret_cast< LPSTREAM* >(wParam) );
            aMsgCtx->aCondition.set( );
        }
        break;

    case MSG_FLUSHCLIPBOARD:
        {
            MsgCtx* aMsgCtx = reinterpret_cast< MsgCtx* >( lParam );
            OSL_ASSERT( aMsgCtx );

            aMsgCtx->hr = pImpl->onFlushClipboard( );
            aMsgCtx->aCondition.set( );
        }
        break;

    case MSG_REGCLIPVIEWER:
        {
            MsgCtx* aMsgCtx = reinterpret_cast< MsgCtx* >( lParam );
            OSL_ASSERT( aMsgCtx );

            pImpl->onRegisterClipViewer( reinterpret_cast<CMtaOleClipboard::LPFNC_CLIPVIEWER_CALLBACK_t>(wParam) );
            aMsgCtx->aCondition.set( );
        }
        break;

    case WM_CHANGECBCHAIN:
        lResult = pImpl->onChangeCBChain(
            reinterpret_cast< HWND >( wParam ), reinterpret_cast< HWND >( lParam ) );
        break;

    case WM_DRAWCLIPBOARD:
        lResult = pImpl->onDrawClipboard( );
        break;

    case MSG_SHUTDOWN:
        DestroyWindow( pImpl->m_hwndMtaOleReqWnd );
        break;

    // force the sta thread to end
    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;

    default:
        lResult = DefWindowProcA( hWnd, uMsg, wParam, lParam );
        break;
    }

    return lResult;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

void CMtaOleClipboard::createMtaOleReqWnd( )
{
    WNDCLASSEXA  wcex;

    HINSTANCE hInst = GetModuleHandleA( CLIPSRV_DLL_NAME );
    OSL_ENSURE( NULL != hInst, "The name of the clipboard service dll must have changed" );

    ZeroMemory( &wcex, sizeof( WNDCLASSEXA ) );

    wcex.cbSize         = sizeof(WNDCLASSEXA);
    wcex.style          = 0;
    wcex.lpfnWndProc    = static_cast< WNDPROC >( CMtaOleClipboard::mtaOleReqWndProc );
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInst;
    wcex.hIcon          = NULL;
    wcex.hCursor        = NULL;
    wcex.hbrBackground  = NULL;
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = g_szWndClsName;
    wcex.hIconSm        = NULL;

    m_MtaOleReqWndClassAtom = RegisterClassExA( &wcex );

    if ( 0 != m_MtaOleReqWndClassAtom )
        m_hwndMtaOleReqWnd = CreateWindowA(
            g_szWndClsName, NULL, 0, 0, 0, 0, 0, NULL, NULL, hInst, NULL );
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

unsigned int CMtaOleClipboard::run( )
{
    #if OSL_DEBUG_LEVEL > 0
    HRESULT hr =
    #endif
        OleInitialize( NULL );
    OSL_ASSERT( SUCCEEDED( hr ) );

    createMtaOleReqWnd( );

    unsigned int nRet;

    if ( IsWindow( m_hwndMtaOleReqWnd ) )
    {
        if ( NULL != m_hEvtThrdReady )
            SetEvent( m_hEvtThrdReady );

        // pumping messages
        MSG msg;
        while( GetMessageA( &msg, NULL, 0, 0 ) )
            DispatchMessageA( &msg );

        nRet = 0;
    }
    else
        nRet = ~0U;

    OleUninitialize( );

    return nRet;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

unsigned int WINAPI CMtaOleClipboard::oleThreadProc( LPVOID pParam )
{
    CMtaOleClipboard* pInst =
        reinterpret_cast<CMtaOleClipboard*>( pParam );
    OSL_ASSERT( NULL != pInst );

    return pInst->run( );
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

unsigned int WINAPI CMtaOleClipboard::clipboardChangedNotifierThreadProc( LPVOID pParam )
{
    CMtaOleClipboard* pInst = reinterpret_cast< CMtaOleClipboard* >( pParam );
    OSL_ASSERT( NULL != pInst );

    CoInitialize( NULL );

    // assuming we don't need a lock for
    // a boolean variable like m_bRun...
    while ( pInst->m_bRunClipboardNotifierThread )
    {
        // wait for clipboard changed or terminate event
        WaitForMultipleObjects( 2, pInst->m_hClipboardChangedNotifierEvents, false, INFINITE );

        ClearableMutexGuard aGuard( pInst->m_ClipboardChangedEventCountMutex );

        if ( pInst->m_ClipboardChangedEventCount > 0 )
        {
            pInst->m_ClipboardChangedEventCount--;
            if ( 0 == pInst->m_ClipboardChangedEventCount )
                ResetEvent( pInst->m_hClipboardChangedEvent );

            aGuard.clear( );

            // nobody should touch m_pfncClipViewerCallback while we do
            MutexGuard aClipViewerGuard( pInst->m_pfncClipViewerCallbackMutex );

            // notify all clipboard listener
            if ( pInst->m_pfncClipViewerCallback )
                pInst->m_pfncClipViewerCallback( );
        }
        else
            aGuard.clear( );
    }

    CoUninitialize( );

    return ( 0 );
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

bool CMtaOleClipboard::WaitForThreadReady( ) const
{
    bool bRet = false;

    if ( NULL != m_hEvtThrdReady )
    {
        DWORD dwResult = WaitForSingleObject(
            m_hEvtThrdReady, MAX_WAITTIME );
        bRet = ( dwResult == WAIT_OBJECT_0 );
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
