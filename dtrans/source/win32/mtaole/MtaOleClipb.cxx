/*************************************************************************
 *
 *  $RCSfile: MtaOleClipb.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-16 16:32:57 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma warning( disable : 4290 ) // c++ exception specification ignored
#pragma warning( disable : 4786 ) // identifier was truncated to 'number'
                                   // characters in the debug information

#define UNICODE

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include <systools/win32/user9x.h>

#include "..\..\inc\MtaOleClipb.hxx"

#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif

#include <wchar.h>
#include <process.h>

//----------------------------------------------------------------
//  namespace directives
//----------------------------------------------------------------

using osl::Condition;

//----------------------------------------------------------------
//  defines
//----------------------------------------------------------------

#define HIDDEN_CB_WNDCLS_NAME L"MtaOleReqWnd_920896680C9449"
#define CLIPSERVICE_DLL_NAME "sysdtrans.dll"

//--------------------------------------------------------
// messages constants
//--------------------------------------------------------

const sal_uInt32 MSG_SETCLIPBOARD               = WM_USER + 0x0001;
const sal_uInt32 MSG_GETCLIPBOARD               = WM_USER + 0x0002;
const sal_uInt32 MSG_REGCLIPVIEWER              = WM_USER + 0x0003;
const sal_uInt32 MSG_FLUSHCLIPBOARD             = WM_USER + 0x0004;
const sal_uInt32 MSG_SHUTDOWN                   = WM_USER + 0x0006;

const sal_uInt32 MAX_WAITTIME                   = 60000;
const sal_uInt32 MAX_OPCOMPLET_WAITTIME         = 30;
const sal_uInt32 MAX_WAIT_SHUTDOWN              = 30000;
const sal_uInt32 MAX_CLIPEVENT_PROCESSING_TIME  = 5000;

const sal_Bool MANUAL_RESET                     = sal_True;
const sal_Bool AUTO_RESET                       = sal_False;
const sal_Bool INIT_NONSIGNALED                 = sal_False;

//----------------------------------------------------------------
//
//----------------------------------------------------------------

typedef struct _MsgCtx
{
    Condition*  aCondition;
    HRESULT*    hr;
} MsgCtx;

//----------------------------------------------------------------
//  static member initialization
//----------------------------------------------------------------

CMtaOleClipboard* CMtaOleClipboard::s_theMtaOleClipboardInst = NULL;

//--------------------------------------------------------------------
// marshal an IDataObject
//--------------------------------------------------------------------

inline
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

inline
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
        CoInitialize( NULL );
    }

    ~CAutoComInit( )
    {
        CoUninitialize( );
    }
};

//--------------------------------------------------------------------
// ctor
//--------------------------------------------------------------------

CMtaOleClipboard::CMtaOleClipboard( ) :
    m_hOleThread( NULL ),
    m_uOleThreadId( 0 ),
    m_hEvtThrdReady( NULL ),
    m_hwndMtaOleReqWnd( NULL ),
    m_hwndNextClipViewer( NULL ),
    m_pfncClipViewerCallback( NULL )
{
    // signals that the thread was successfully set up
    m_hEvtThrdReady  = CreateEventA( 0, MANUAL_RESET, INIT_NONSIGNALED, NULL );

    OSL_ASSERT( NULL != m_hEvtThrdReady );

    s_theMtaOleClipboardInst = this;

    m_hOleThread = (HANDLE)_beginthreadex(
        NULL, 0, CMtaOleClipboard::oleThreadProc, this, 0, &m_uOleThreadId );
    OSL_ASSERT( NULL != m_hOleThread );
}

//--------------------------------------------------------------------
// dtor
//--------------------------------------------------------------------

CMtaOleClipboard::~CMtaOleClipboard( )
{
    // block calling threads out
    if ( NULL != m_hEvtThrdReady )
        ResetEvent( m_hEvtThrdReady );

    // end the thread
    // because DestroyWindow can only be called
    // from within the thread that created the window
    sendMessage( MSG_SHUTDOWN,
                 static_cast< WPARAM >( 0 ),
                 static_cast< LPARAM >( 0 ) );

    // wait for thread shutdown
    sal_uInt32 dwResult =
        WaitForSingleObject( m_hOleThread, MAX_WAIT_SHUTDOWN );
    OSL_ENSURE( dwResult == WAIT_OBJECT_0, "OleThread could not terminate" );

    if ( NULL != m_hOleThread )
        CloseHandle( m_hOleThread );

    if ( NULL != m_hEvtThrdReady )
        CloseHandle( m_hEvtThrdReady );

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
        return E_FAIL;

    CAutoComInit comAutoInit;

    HRESULT hr;

    // we don't need to post the request if we are
    // the ole thread self
    if ( GetCurrentThreadId( ) == m_uOleThreadId )
        OSL_ENSURE( sal_False, "flushClipboard from within the OleThread called" );
    else
    {
        Condition aCondt;
        MsgCtx    aMsgCtx;

        aMsgCtx.aCondition = &aCondt;
        aMsgCtx.hr         = &hr;

        postMessage( MSG_FLUSHCLIPBOARD,
                     static_cast< WPARAM >( 0 ),
                     reinterpret_cast< LPARAM >( &aMsgCtx ) );

        TimeValue tv;
        tv.Seconds = MAX_OPCOMPLET_WAITTIME;
        tv.Nanosec = 0;

        if ( aCondt.wait( &tv ) )
        {
            OSL_ENSURE( sal_False, "Operation timeout" );
            hr = E_FAIL;
        }
    }

    return hr;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

HRESULT CMtaOleClipboard::getClipboard( IDataObject** ppIDataObject )
{
    if ( !WaitForThreadReady( ) )
        return E_FAIL;

    if ( NULL == ppIDataObject )
        return E_INVALIDARG;

    CAutoComInit comAutoInit;

    LPSTREAM lpStream;
    HRESULT  hr;

    *ppIDataObject = NULL;

    // we don't need to post the request if we are
    // the ole thread self
    if ( GetCurrentThreadId( ) == m_uOleThreadId )
        hr = static_cast<HRESULT>( onGetClipboard( &lpStream ) );
    else
    {
        Condition aCondt;
        MsgCtx    aMsgCtx;

        aMsgCtx.aCondition = &aCondt;
        aMsgCtx.hr         = &hr;

        postMessage( MSG_GETCLIPBOARD,
                     reinterpret_cast< WPARAM >( &lpStream ),
                     reinterpret_cast< LPARAM >( &aMsgCtx ) );

        TimeValue tv;
        tv.Seconds = MAX_OPCOMPLET_WAITTIME;
        tv.Nanosec = 0;

        if ( aCondt.wait( &tv ) )
        {
            OSL_ENSURE( sal_False, "Operation timeout" );
            hr = E_FAIL;
        }
    }

    if ( SUCCEEDED( hr ) )
        hr = UnmarshalIDataObjectAndReleaseStream( lpStream, ppIDataObject );

    return hr;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

HRESULT CMtaOleClipboard::setClipboard( IDataObject* pIDataObject )
{
    if ( !WaitForThreadReady( ) )
        return E_FAIL;

    CAutoComInit comAutoInit;

    HRESULT hr;

    // we don't need to post the request if we are
    // the ole thread self
    if ( GetCurrentThreadId( ) == m_uOleThreadId )
        OSL_ENSURE( sal_False, "setClipboard from within the OleThread called" );
    else
    {
        Condition aCondt;
        MsgCtx    aMsgCtx;

        aMsgCtx.aCondition = &aCondt;
        aMsgCtx.hr         = &hr;

        postMessage( MSG_SETCLIPBOARD,
                     reinterpret_cast< WPARAM >( pIDataObject ),
                     reinterpret_cast< LPARAM >( &aMsgCtx ) );

        TimeValue tv;
        tv.Seconds = MAX_OPCOMPLET_WAITTIME;
        tv.Nanosec = 0;

        if ( aCondt.wait( &tv ) )
        {
            OSL_ENSURE( sal_False, "Operation timeout" );
            hr = E_FAIL;
        }
    }

    return hr;
}

//--------------------------------------------------------------------
// register a clipboard viewer
//--------------------------------------------------------------------

sal_Bool CMtaOleClipboard::registerClipViewer( LPFNC_CLIPVIEWER_CALLBACK_t pfncClipViewerCallback )
{
    if ( !WaitForThreadReady( ) )
        return sal_False;

    sal_Bool bRet = sal_False;

    if ( GetCurrentThreadId( ) == m_uOleThreadId )
        OSL_ENSURE( sal_False, "registerClipViewer from within the OleThread called" );
    else
    {
        Condition aCondt;
        MsgCtx    aMsgCtx;
        HRESULT   hr;

        aMsgCtx.aCondition = &aCondt;
        aMsgCtx.hr         = &hr;

        postMessage( MSG_REGCLIPVIEWER,
                     reinterpret_cast<WPARAM>( pfncClipViewerCallback ),
                     reinterpret_cast<LPARAM>( &aMsgCtx ) );

        TimeValue tv;
        tv.Seconds = MAX_OPCOMPLET_WAITTIME;
        tv.Nanosec = 0;

        if ( aCondt.wait( &tv ) )
        {
            OSL_ENSURE( sal_False, "Operation timeout" );
            hr = E_FAIL;
        }
    }

    return bRet;
}

//--------------------------------------------------------------------
// register a clipboard viewer
//--------------------------------------------------------------------

sal_Bool CMtaOleClipboard::onRegisterClipViewer( LPFNC_CLIPVIEWER_CALLBACK_t pfncClipViewerCallback )
{
    if ( !IsWindow( m_hwndMtaOleReqWnd ) )
        return sal_False;

    sal_Bool bRet = sal_True;

    // register if not yet done
    if ( ( NULL != pfncClipViewerCallback ) && ( NULL == m_pfncClipViewerCallback ) )
    {
        // SetClipboardViewer sends a WM_DRAWCLIPBOARD message we ignore
        // this message if we register ourself as clip viewer
        m_bInRegisterClipViewer = sal_True;
        m_hwndNextClipViewer = SetClipboardViewer( m_hwndMtaOleReqWnd );
        m_bInRegisterClipViewer = sal_False;

        // if there is no other cb-viewer the
        // return value is NULL!!!
        bRet = IsWindow( m_hwndNextClipViewer );

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
    OSL_ASSERT( NULL != ppStream );

    IDataObject* pIDataObject;

    // forward the request to the OleClipboard
    HRESULT hr = OleGetClipboard( &pIDataObject );
    if ( SUCCEEDED( hr ) )
        hr = MarshalIDataObjectInStream( pIDataObject, ppStream );

    return static_cast< LRESULT >( hr );
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
        DWORD dwResult;
        SendMessageTimeoutA(
            m_hwndNextClipViewer,
            WM_CHANGECBCHAIN,
            reinterpret_cast<WPARAM>(hWndRemove),
            reinterpret_cast<LPARAM>(hWndNext),
            SMTO_BLOCK,
            MAX_CLIPEVENT_PROCESSING_TIME,
            &dwResult );
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
    if ( !m_bInRegisterClipViewer && (NULL != m_pfncClipViewerCallback) )
        m_pfncClipViewerCallback( );

    // foward the message to the next viewer in the chain
    if ( IsWindow( m_hwndNextClipViewer ) )
    {
        DWORD dwResult;
        SendMessageTimeoutA(
            m_hwndNextClipViewer,
            WM_DRAWCLIPBOARD,
            static_cast< WPARAM >( 0 ),
            static_cast< LPARAM >( 0 ),
            SMTO_BLOCK,
            MAX_CLIPEVENT_PROCESSING_TIME,
            &dwResult );
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

sal_Bool CMtaOleClipboard::postMessage( UINT msg, WPARAM wParam, LPARAM lParam )
{
    return PostMessageA( m_hwndMtaOleReqWnd, msg, wParam, lParam );
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

    MsgCtx* aMsgCtx = reinterpret_cast< MsgCtx* >( lParam );

    switch( uMsg )
    {
    case MSG_SETCLIPBOARD:
        *aMsgCtx->hr = pImpl->onSetClipboard( reinterpret_cast< IDataObject* >(wParam) );
        aMsgCtx->aCondition->set( );
        break;

    case MSG_GETCLIPBOARD:
        *aMsgCtx->hr = pImpl->onGetClipboard( reinterpret_cast< LPSTREAM* >(wParam) );
        aMsgCtx->aCondition->set( );
        break;

    case MSG_FLUSHCLIPBOARD:
        *aMsgCtx->hr = pImpl->onFlushClipboard( );
        aMsgCtx->aCondition->set( );
        break;

    case MSG_REGCLIPVIEWER:
        pImpl->onRegisterClipViewer( reinterpret_cast<LPFNC_CLIPVIEWER_CALLBACK_t>(wParam) );
        aMsgCtx->aCondition->set( );
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

    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;

    default:
        return DefWindowProc( hWnd, uMsg, wParam, lParam );
    }

    return lResult;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

void CMtaOleClipboard::createMtaOleReqWnd( )
{
    WNDCLASSEXW  wcex;
    sal_Unicode szWndClsName[] = HIDDEN_CB_WNDCLS_NAME;

    HINSTANCE hInst = GetModuleHandleA( CLIPSERVICE_DLL_NAME );
    OSL_ENSURE( NULL != hInst, "The name of the clipboard service dll must have changed" );

    ZeroMemory( &wcex, sizeof( WNDCLASSEX ) );

    wcex.cbSize        = sizeof( WNDCLASSEX );
    wcex.lpfnWndProc   = static_cast< WNDPROC >( CMtaOleClipboard::mtaOleReqWndProc );
    wcex.hInstance     = hInst;
    wcex.lpszClassName = szWndClsName;

    ATOM atom = RegisterClassExW( &wcex );
    if ( 0 != atom )
        m_hwndMtaOleReqWnd = CreateWindowExW(
            0, szWndClsName, TEXT(""), 0, 0, 0, 0, 0, NULL, NULL, hInst, NULL );
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

unsigned int CMtaOleClipboard::run( )
{
    HRESULT hr = OleInitialize( NULL );
    OSL_ASSERT( SUCCEEDED( hr ) );

    createMtaOleReqWnd( );

    unsigned int nRet;

    if ( IsWindow( m_hwndMtaOleReqWnd ) )
    {
        if ( NULL != m_hEvtThrdReady )
            SetEvent( m_hEvtThrdReady );

        // pumping messages
        MSG msg;
        while( GetMessage( &msg, NULL, 0, 0 ) )
            DispatchMessage( &msg );

        nRet = 0;
    }
    else
        nRet = -1;

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

inline
sal_Bool CMtaOleClipboard::WaitForThreadReady( ) const
{
    sal_Bool bRet = sal_False;

    if ( NULL != m_hEvtThrdReady )
    {
        DWORD dwResult = WaitForSingleObject(
            m_hEvtThrdReady, MAX_WAITTIME );
        bRet = ( dwResult == WAIT_OBJECT_0 );
    }

    return bRet;
}

