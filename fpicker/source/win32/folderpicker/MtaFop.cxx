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

#include <osl/diagnose.h>
#include <osl/thread.h>

#include "MtaFop.hxx"
#include <wchar.h>
#include <process.h>
#include "../misc/resourceprovider.hxx"

#include <systools/win32/comtools.hxx>


const sal_uInt32 MSG_BROWSEFORFOLDER = WM_USER + 1;
const sal_uInt32 MSG_SHUTDOWN        = WM_USER + 2;

const sal_uInt32 MAX_WAITTIME        = 2000; // msec

const bool MANUAL_RESET     = true;
const bool AUTO_RESET       = false;
const bool INIT_NONSIGNALED = false;

#define FOLDERPICKER_TITLE            500
#define FOLDER_PICKER_DEF_DESCRIPTION 501

namespace
{
    const wchar_t* const FOLDERPICKER_SRV_DLL_NAME = L"fps.dll";
    const wchar_t g_szWndClsName[]                 = L"FopStaReqWnd###";
    const wchar_t* const CURRENT_INSTANCE          = L"CurrInst";

    struct RequestContext
    {
        HANDLE   hEvent;
        bool bRet;
    };

    inline bool InitializeRequestContext( RequestContext* aRequestContext )
    {
        OSL_ASSERT( aRequestContext );

        aRequestContext->hEvent = CreateEventW(
            nullptr, AUTO_RESET, INIT_NONSIGNALED, nullptr );

        aRequestContext->bRet = false;

        return ( nullptr != aRequestContext->hEvent );
    }

    inline void DeinitializeRequestContext( RequestContext* aRequestContext )
    {
        OSL_ASSERT( aRequestContext && aRequestContext->hEvent );
        CloseHandle( aRequestContext->hEvent );
    }


    // Determine if current thread is
    // an MTA or STA thread

    bool IsMTA()
    {
        HRESULT hr = CoInitialize(nullptr);

        if (RPC_E_CHANGED_MODE == hr)
            return true;

        if(SUCCEEDED(hr))
            CoUninitialize();

        return false;
    }
}


//  static member initialization


ATOM CMtaFolderPicker::s_ClassAtom = 0;
osl::Mutex CMtaFolderPicker::s_Mutex;
sal_Int32 CMtaFolderPicker::s_StaRequestWndRegisterCount = 0;


// ctor


CMtaFolderPicker::CMtaFolderPicker( sal_uInt32 Flags ) :
    m_hStaThread( nullptr ),
    m_uStaThreadId( 0 ),
    m_hEvtThrdReady( nullptr ),
    m_hwndStaRequestWnd( nullptr )
{
    m_hInstance = GetModuleHandleW( FOLDERPICKER_SRV_DLL_NAME );
    OSL_ENSURE( m_hInstance, "The name of the FolderPicker service dll must have changed" );

    ZeroMemory( &m_bi, sizeof( m_bi ) );

    // !!!!!!!!!!!!!!!!!  IMPORTANT !!!!!!!!!!!!!!!!!!!

    // Remember: This HACK prevents you from stepping
    // through your code in the debugger because if you
    // set a break point in the ctor here the debugger
    // may become the owner of the FolderBrowse dialog
    // and so it seems that the Visual Studio and the
    // office are hanging
    m_bi.hwndOwner = GetForegroundWindow( );

    /*
        Flag                Available
        --------------------------------
        BIF_EDITBOX         Version 4.71
        BIF_NEWDIALOGSTYLE  Version 5.0
        BIF_SHAREABLE       Version 5.0
        BIF_VALIDATE        Version 4.71

        Version 4.71 - Internet Explorer 4.0
        Version 5.0  - Internet Explorer 5.0
                       Windows 2000
    */
    m_bi.ulFlags = Flags;

    m_bi.lpfn    = CMtaFolderPicker::FolderPickerCallback;
    m_bi.lParam  = reinterpret_cast< LPARAM >( this );

    m_dialogTitle = CResourceProvider::getResString(FOLDERPICKER_TITLE);
    m_Description = CResourceProvider::getResString(FOLDER_PICKER_DEF_DESCRIPTION);

    // signals that the thread was successfully set up
    m_hEvtThrdReady  = CreateEventW(
        nullptr,
        MANUAL_RESET,
        INIT_NONSIGNALED,
        nullptr );

    if ( m_hEvtThrdReady )
    {
        // setup the sta thread
        m_hStaThread = reinterpret_cast<HANDLE>(_beginthreadex(
            nullptr,
            0,
            CMtaFolderPicker::StaThreadProc,
            this,
            0,
            &m_uStaThreadId ));

        OSL_ASSERT( m_hStaThread );
    }

    OSL_ASSERT( m_hEvtThrdReady );
}


// dtor


CMtaFolderPicker::~CMtaFolderPicker( )
{
    // only if the is a valid event handle
    // there may also be a thread a hidden
    // target request window and so on
    // see ctor
    if ( m_hEvtThrdReady )
    {
        // block calling threads because we
        // are about to shutdown
        ResetEvent( m_hEvtThrdReady );

        // force the destruction of the sta thread request window
        // and the end of the thread
        // remember: DestroyWindow may only be called from within
        // the thread that created the window
        if ( IsWindow( m_hwndStaRequestWnd ) )
        {
            SendMessageW( m_hwndStaRequestWnd, MSG_SHUTDOWN, 0, 0 );

            // we place unregister class here because
            // if we have a valid window we must have
            // successfully registered a window class
            // if the creation of the window itself
            // failed after registering the window
            // class we have unregistered it immediately
            // in createStaRequestWindow below
            UnregisterStaRequestWindowClass( );
        }

        if ( m_hStaThread )
        {
            // wait for thread shutdown
            sal_uInt32 dwResult = WaitForSingleObject( m_hStaThread, MAX_WAITTIME );
            OSL_ENSURE( dwResult == WAIT_OBJECT_0, "sta thread could not terminate" );

            // terminate the thread if it
            // doesn't shutdown itself
            if ( WAIT_OBJECT_0 != dwResult )
                TerminateThread(
                    m_hStaThread, sal::static_int_cast< DWORD >(-1) );

            CloseHandle( m_hStaThread );
        }

        CloseHandle( m_hEvtThrdReady );
    }
}


bool CMtaFolderPicker::browseForFolder( )
{
    bool bRet = false;

    if (IsMTA())
    {

        OSL_ASSERT( m_hEvtThrdReady );

        if ( WaitForSingleObject( m_hEvtThrdReady, MAX_WAITTIME ) != WAIT_OBJECT_0 )
        {
            OSL_FAIL( "sta thread not ready" );
            return false;
        }

        RequestContext aReqCtx;

        if ( !InitializeRequestContext( &aReqCtx ) )
        {
            OSL_ASSERT( false );
            return false;
        }

        // marshall request into the sta thread
        BOOL const ret = PostMessageW(
            m_hwndStaRequestWnd,
            MSG_BROWSEFORFOLDER,
            0,
            reinterpret_cast< LPARAM >( &aReqCtx ) );
        SAL_WARN_IF(0 == ret, "fpicker", "ERROR: PostMessage() failed!");

        // waiting for the event to be signaled or
        // window messages so that we don't block
        // our parent window

        bool bContinue = true;

        while ( bContinue )
        {
            DWORD dwResult = MsgWaitForMultipleObjects(
                1, &aReqCtx.hEvent, false, INFINITE, QS_ALLEVENTS );

            switch ( dwResult )
            {
            // the request context event is signaled
            case WAIT_OBJECT_0:
                bContinue = false;
                break;

            // a window message has arrived
            case WAIT_OBJECT_0 + 1:
                {
                    // dispatching all messages but we expect to
                    // receive only paint or timer messages that's
                    // why we don't need to call TranslateMessage or
                    // TranslateAccelerator, because keyboard or
                    // mouse messages are for the FolderPicker which
                    // is in the foreground and should not arrive here
                    MSG msg;
                    while ( PeekMessageW( &msg, nullptr, 0, 0, PM_REMOVE ) )
                        DispatchMessageW(&msg);
                }
                break;

            // should not happen
            default:
                OSL_ASSERT( false );
            }
        }

        /*sal_Bool*/ bRet = aReqCtx.bRet;
        DeinitializeRequestContext( &aReqCtx );
    }
    else
    {
        bRet = onBrowseForFolder();
    }

    return bRet;
}


void SAL_CALL CMtaFolderPicker::setDisplayDirectory( const OUString& aDirectory )
{
    m_displayDir = aDirectory;
}


OUString SAL_CALL CMtaFolderPicker::getDisplayDirectory( )
{
    return m_displayDir;
}


OUString SAL_CALL CMtaFolderPicker::getDirectory( )
{
    return m_SelectedDir;
}


void SAL_CALL CMtaFolderPicker::setDescription( const OUString& aDescription )
{
    m_Description = aDescription;
}


void SAL_CALL CMtaFolderPicker::setTitle( const OUString& aTitle )
{
    m_dialogTitle = aTitle;
}


OUString SAL_CALL CMtaFolderPicker::getTitle( )
{
    return m_dialogTitle;
}


// XCancellable


void SAL_CALL CMtaFolderPicker::cancel( )
{
    if ( IsWindow( m_hwnd ) )
    {
        // simulate a mouse click to the
        // cancel button
        BOOL const ret = PostMessageW(
            m_hwnd,
            WM_COMMAND,
            MAKEWPARAM( IDCANCEL, BN_CLICKED ),
            reinterpret_cast<LPARAM>(GetDlgItem( m_hwnd, IDCANCEL )) );
        SAL_WARN_IF(0 == ret, "fpicker", "ERROR: PostMessage() failed!");
    }
}


bool SAL_CALL CMtaFolderPicker::onBrowseForFolder( )
{
    bool bRet;
    LPITEMIDLIST lpiid;

    // pre SHBrowseFroFolder

    m_bi.pidlRoot       = nullptr;
    m_bi.pszDisplayName = SAL_W(m_pathBuff);

    if ( m_Description.getLength( ) )
        m_bi.lpszTitle = SAL_W(m_Description.getStr( ));

    lpiid = SHBrowseForFolderW( &m_bi );
    bRet = ( nullptr != lpiid );

    // post SHBrowseForFolder

    m_SelectedDir = getPathFromItemIdList( lpiid );
    releaseItemIdList( lpiid );

    return bRet;
}


void SAL_CALL CMtaFolderPicker::releaseItemIdList( LPITEMIDLIST lpItemIdList )
{
    sal::systools::COMReference<IMalloc> pIMalloc;
    SHGetMalloc(&pIMalloc);
    if (pIMalloc.is())
    {
        pIMalloc->Free(lpItemIdList);
        lpItemIdList = nullptr;
    }
}


LPITEMIDLIST SAL_CALL CMtaFolderPicker::getItemIdListFromPath( const OUString& aDirectory )
{
    // parameter checking
    if ( !aDirectory.getLength( ) )
        return nullptr;

    LPITEMIDLIST lpItemIdList(nullptr);

    sal::systools::COMReference<IShellFolder> pIShellFolder;
    SHGetDesktopFolder(&pIShellFolder);

    if (pIShellFolder.is())
    {
        pIShellFolder->ParseDisplayName(
            nullptr,
            nullptr,
            const_cast<LPWSTR>(SAL_W( aDirectory.getStr( ) )),
            nullptr,
            &lpItemIdList,
            nullptr );
    }

    return lpItemIdList;
}


OUString SAL_CALL CMtaFolderPicker::getPathFromItemIdList( LPCITEMIDLIST lpItemIdList )
{
    OUString path;

    if ( lpItemIdList )
    {
        bool bRet = SHGetPathFromIDListW( lpItemIdList, SAL_W(m_pathBuff) );
        if ( bRet )
            path = m_pathBuff;
    }

    return path;
}


void SAL_CALL CMtaFolderPicker::enableOk( bool bEnable )
{
    OSL_ASSERT( IsWindow( m_hwnd ) );

    SendMessageW(
        m_hwnd,
        BFFM_ENABLEOK,
        static_cast< WPARAM >( 0 ),
        static_cast< LPARAM >( bEnable ) );
}


void SAL_CALL CMtaFolderPicker::setSelection( const OUString& aDirectory )
{
    OSL_ASSERT( IsWindow( m_hwnd ) );

    SendMessageW(
        m_hwnd,
        BFFM_SETSELECTIONW,
        static_cast< WPARAM >( true ),
        reinterpret_cast< LPARAM >( aDirectory.getStr( ) ) );
}


void SAL_CALL CMtaFolderPicker::setStatusText( const OUString& aStatusText )
{
    OSL_ASSERT( IsWindow( m_hwnd ) );

    SendMessageW(
        m_hwnd,
        BFFM_SETSTATUSTEXTW,
        static_cast< WPARAM >( 0 ),
        reinterpret_cast< LPARAM >( aStatusText.getStr( ) ) );
}


void SAL_CALL CMtaFolderPicker::onInitialized( )
{
    LPITEMIDLIST lpiidDisplayDir = getItemIdListFromPath( m_displayDir );

    if ( lpiidDisplayDir )
    {
        SendMessageW(
            m_hwnd,
            BFFM_SETSELECTION,
            (WPARAM)false,
            reinterpret_cast<LPARAM>(lpiidDisplayDir) );

        releaseItemIdList( lpiidDisplayDir );
    }
}


sal_uInt32 CMtaFolderPicker::onValidateFailed()
{
    // to be overwritten by subclasses
    return 1;
}


int CALLBACK CMtaFolderPicker::FolderPickerCallback( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
    CMtaFolderPicker* pImpl = reinterpret_cast< CMtaFolderPicker* >( lpData );
    OSL_ASSERT( pImpl );

    int nRC = 0;

    switch( uMsg )
    {
        case BFFM_INITIALIZED:
            pImpl->m_hwnd = hwnd;
            pImpl->onInitialized( );
            SetWindowTextW( hwnd, SAL_W(pImpl->m_dialogTitle.getStr()) );
        break;

        case BFFM_SELCHANGED:
            pImpl->m_hwnd = hwnd;
            pImpl->onSelChanged(
                pImpl->getPathFromItemIdList(
                    reinterpret_cast< LPITEMIDLIST >( lParam ) ) );
        break;

        case BFFM_VALIDATEFAILEDW:
            nRC = CMtaFolderPicker::onValidateFailed();
            break;

        default:
            OSL_ASSERT( false );
    }

    return nRC;
}


// the window proc


LRESULT CALLBACK CMtaFolderPicker::StaWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LRESULT           lResult = 0;
    CMtaFolderPicker* pImpl   = nullptr;

    /*
        we connect to the belonging class instance of this
        window using SetProp, GetProp etc.
        this may fail if somehow the class instance destroyed
        before the window
    */

    switch( uMsg )
    {
        case WM_CREATE:
            {
                LPCREATESTRUCTW lpcs =
                    reinterpret_cast< LPCREATESTRUCTW >( lParam );

                OSL_ASSERT( lpcs->lpCreateParams );

                // connect the instance handle to the window
                SetPropW( hWnd, CURRENT_INSTANCE, lpcs->lpCreateParams );
            }
            break;

        case WM_NCDESTROY:
                // RemoveProp returns the saved value on success
                pImpl = static_cast< CMtaFolderPicker* >(
                    RemovePropW( hWnd, CURRENT_INSTANCE ) );

                OSL_ASSERT( pImpl && !IsBadReadPtr( pImpl, sizeof( CMtaFolderPicker ) ) );
            break;

        case MSG_BROWSEFORFOLDER:
            {
                RequestContext* aReqCtx = reinterpret_cast< RequestContext* >( lParam );
                OSL_ASSERT( aReqCtx );

                pImpl = static_cast< CMtaFolderPicker* >(
                    GetPropW( hWnd, CURRENT_INSTANCE ) );

                OSL_ASSERT( pImpl && !IsBadReadPtr( pImpl, sizeof( CMtaFolderPicker ) ) );

                aReqCtx->bRet = pImpl->onBrowseForFolder( );
                SetEvent( aReqCtx->hEvent );
            }
            break;

        case MSG_SHUTDOWN:
            pImpl = static_cast< CMtaFolderPicker* >(
                GetPropW( hWnd, CURRENT_INSTANCE ) );

            OSL_ASSERT( pImpl && !IsBadReadPtr( pImpl, sizeof( CMtaFolderPicker ) ) );

            DestroyWindow( pImpl->m_hwndStaRequestWnd );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            lResult = DefWindowProcW( hWnd, uMsg, wParam, lParam );
            break;
    }

    return lResult;
}


bool SAL_CALL CMtaFolderPicker::createStaRequestWindow( )
{
    bool bIsWnd = false;

    if ( RegisterStaRequestWindowClass( ) )
    {
        m_hwndStaRequestWnd = CreateWindowW(
            g_szWndClsName, nullptr,
            0, 0, 0, 0, 0,
            nullptr, nullptr, m_hInstance,
            this // provide the instance of the class
        );

        bIsWnd = IsWindow( m_hwndStaRequestWnd );

        // we do immediately unregister the window class
        // if the creation of the window fails because we
        // don't want to spoil the register class counter
        if ( !bIsWnd )
            UnregisterStaRequestWindowClass( );

        OSL_ENSURE( bIsWnd, "sta request window creation failed" );
    }

    return bIsWnd;
}


unsigned int CMtaFolderPicker::run( )
{
    OSL_ASSERT( m_hEvtThrdReady );

    // setup an sta environment
    HRESULT hr = CoInitialize( nullptr );

    // if we can't setup an sta environment
    // we stop here and return
    if ( FAILED( hr ) )
    {
        OSL_FAIL( "CoInitialize failed" );
        return sal::static_int_cast< unsigned int >(-1);
    }

    unsigned int nRet;

    if ( createStaRequestWindow( ) )
    {
        SetEvent( m_hEvtThrdReady );

        // pumping messages
        MSG msg;
        while( GetMessageW( &msg, nullptr, 0, 0 ) )
            DispatchMessageW( &msg );

        nRet = 0;
    }
    else
    {
        OSL_FAIL( "failed to create sta thread" );
        nRet = sal::static_int_cast< unsigned int >(-1);
    }

    // shutdown sta environment
    CoUninitialize( );

    return nRet;
}


unsigned int WINAPI CMtaFolderPicker::StaThreadProc( LPVOID pParam )
{
    osl_setThreadName("fpicker CMtaFolderPicker::run()");

    CMtaFolderPicker* pInst =
        static_cast<CMtaFolderPicker*>( pParam );

    OSL_ASSERT( pInst );

    HRESULT hr = OleInitialize( nullptr );

    unsigned int    result = pInst->run( );

    if ( SUCCEEDED( hr ) )
        OleUninitialize();

    return result;
}


ATOM SAL_CALL CMtaFolderPicker::RegisterStaRequestWindowClass( )
{
    osl::MutexGuard aGuard( s_Mutex );

    if ( 0 == s_ClassAtom )
    {
        WNDCLASSEXW  wcex;

        ZeroMemory( &wcex, sizeof( wcex ) );

        wcex.cbSize         = sizeof(wcex);
        wcex.style          = 0;
        wcex.lpfnWndProc    = static_cast< WNDPROC >( CMtaFolderPicker::StaWndProc );
        wcex.cbClsExtra     = 0;
        wcex.cbWndExtra     = 0;
        wcex.hInstance      = m_hInstance;
        wcex.hIcon          = nullptr;
        wcex.hCursor        = nullptr;
        wcex.hbrBackground  = nullptr;
        wcex.lpszMenuName   = nullptr;
        wcex.lpszClassName  = g_szWndClsName;
        wcex.hIconSm        = nullptr;

        s_ClassAtom = RegisterClassExW( &wcex );
        OSL_ASSERT( s_ClassAtom );
    }

    // increment the register class counter
    // so that we keep track of the number
    // of class registrations
    if ( 0 != s_ClassAtom )
        s_StaRequestWndRegisterCount++;

    return s_ClassAtom;
}


void SAL_CALL CMtaFolderPicker::UnregisterStaRequestWindowClass( )
{
    osl::MutexGuard aGuard( s_Mutex );

    OSL_ASSERT( 0 != s_ClassAtom );

    // update the register class counter
    // and unregister the window class if
    // counter drops to zero
    if ( 0 != s_ClassAtom )
    {
        s_StaRequestWndRegisterCount--;
        OSL_ASSERT( s_StaRequestWndRegisterCount >= 0 );
    }

    if ( 0 == s_StaRequestWndRegisterCount )
    {
        UnregisterClass( MAKEINTATOM(s_ClassAtom), m_hInstance );

        s_ClassAtom = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
