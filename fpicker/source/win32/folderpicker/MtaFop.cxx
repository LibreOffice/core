/*************************************************************************
 *
 *  $RCSfile: MtaFop.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-20 17:12:25 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif

#include "MtaFop.hxx"
#include <wchar.h>
#include <process.h>
#include <comdef.h>

#ifndef _RESOURCEPROVIDER_HXX_
#include "..\misc\resourceprovider.hxx"
#endif

//----------------------------------------------------------------
//  namespace directives
//----------------------------------------------------------------

using rtl::OUString;
using osl::Condition;

//--------------------------------------------------------
// messages constants
//--------------------------------------------------------

const sal_uInt32 MSG_BROWSEFORFOLDER = WM_USER + 1;
const sal_uInt32 MSG_SHUTDOWN        = WM_USER + 2;

const sal_uInt32 MAX_WAITTIME        = 2000; // msec

const sal_Bool MANUAL_RESET     = sal_True;
const sal_Bool AUTO_RESET       = sal_False;
const sal_Bool INIT_NONSIGNALED = sal_False;

//----------------------------------------------------------------
//  defines
//----------------------------------------------------------------

namespace
{
    const char* FOLDERPICKER_SRV_DLL_NAME = "fop.dll";
    const char g_szWndClsName[]           = "FopStaReqWnd###";
    const char* CURRENT_INSTANCE          = "CurrInst";

    typedef struct _RequestContext
    {
        HANDLE   hEvent;
        sal_Bool bRet;
    } RequestContext;

    inline sal_Bool InitializeRequestContext( RequestContext* aRequestContext )
    {
        OSL_ASSERT( aRequestContext );

        aRequestContext->hEvent = CreateEventA(
            0, AUTO_RESET, INIT_NONSIGNALED, NULL );

        aRequestContext->bRet = sal_False;

        return ( 0 != aRequestContext->hEvent );
    }

    inline void DeinitializeRequestContext( RequestContext* aRequestContext )
    {
        OSL_ASSERT( aRequestContext && aRequestContext->hEvent );
        CloseHandle( aRequestContext->hEvent );
    }

    //-------------------------------
    // Determine if current thread is
    // an MTA or STA thread
    //-------------------------------
    bool IsMTA()
    {
        HRESULT hr = CoInitialize(NULL);

        if (RPC_E_CHANGED_MODE == hr)
            return true;

        if(SUCCEEDED(hr))
            CoUninitialize();

        return false;
    }
}

//----------------------------------------------------------------
//  static member initialization
//----------------------------------------------------------------

ATOM CMtaFolderPicker::s_ClassAtom = 0;
osl::Mutex CMtaFolderPicker::s_Mutex;
sal_Int32 CMtaFolderPicker::s_StaRequestWndRegisterCount = 0;

//--------------------------------------------------------------------
// ctor
//--------------------------------------------------------------------

CMtaFolderPicker::CMtaFolderPicker( sal_uInt32 Flags ) :
    m_hStaThread( NULL ),
    m_uStaThreadId( 0 ),
    m_hEvtThrdReady( NULL ),
    m_hwndStaRequestWnd( NULL )
{
    m_hInstance = GetModuleHandleA( FOLDERPICKER_SRV_DLL_NAME );
    OSL_ENSURE( m_hInstance, "The name of the FolderPicker service dll must have changed" );

    ZeroMemory( &m_bi, sizeof( m_bi ) );

    // !!!!!!!!!!!!!!!!!  IMPORTANT !!!!!!!!!!!!!!!!!!!
    //
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

    //---------------------------------------
    // read the default strings for title and
    // description from a resource file

    CResourceProvider ResProvider;

    m_dialogTitle = ResProvider.getResString( 500 );
    m_Description = ResProvider.getResString( 501 );

    // signals that the thread was successfully set up
    m_hEvtThrdReady  = CreateEventA(
        0,
        MANUAL_RESET,
        INIT_NONSIGNALED,
        NULL );

    if ( m_hEvtThrdReady )
    {
        // setup the sta thread
        m_hStaThread = (HANDLE)_beginthreadex(
            NULL,
            0,
            CMtaFolderPicker::StaThreadProc,
            this,
            0,
            &m_uStaThreadId );

        OSL_ASSERT( m_hStaThread );
    }

    OSL_ASSERT( m_hEvtThrdReady );
}

//--------------------------------------------------------------------
// dtor
//--------------------------------------------------------------------

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
        // remeber: DestroyWindow may only be called from within
        // the thread that created the window
        if ( IsWindow( m_hwndStaRequestWnd ) )
        {
            SendMessageA( m_hwndStaRequestWnd, MSG_SHUTDOWN, 0, 0 );

            // we place unregister class here because
            // if we have a valid window we must have
            // sucessfully registered a window class
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
                TerminateThread( m_hStaThread, -1 );

            CloseHandle( m_hStaThread );
        }

        CloseHandle( m_hEvtThrdReady );
    }
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

sal_Bool CMtaFolderPicker::browseForFolder( )
{
    sal_Bool bRet = sal_False;

    if (IsMTA())
    {

        OSL_ASSERT( m_hEvtThrdReady );

        if ( WaitForSingleObject( m_hEvtThrdReady, MAX_WAITTIME ) != WAIT_OBJECT_0 )
        {
            OSL_ENSURE( sal_False, "sta thread not ready" );
            return sal_False;
        }

        RequestContext aReqCtx;

        if ( !InitializeRequestContext( &aReqCtx ) )
        {
            OSL_ASSERT( sal_False );
            return sal_False;
        }

        // marshall request into the sta thread
        PostMessageA(
            m_hwndStaRequestWnd,
            MSG_BROWSEFORFOLDER,
            0,
            reinterpret_cast< LPARAM >( &aReqCtx ) );

        // waiting for the event to be signaled or
        // window messages so that we don't block
        // our parent window

        sal_Bool bContinue = sal_True;

        while ( bContinue )
        {
            DWORD dwResult = MsgWaitForMultipleObjects(
                1, &aReqCtx.hEvent, FALSE, INFINITE, QS_ALLEVENTS );

            switch ( dwResult )
            {
            // the request context event is signaled
            case WAIT_OBJECT_0:
                bContinue = sal_False;
                break;

            // a window message has arrived
            case WAIT_OBJECT_0 + 1:
                {
                    // dispatching all messages but we expect to
                    // receive only paint or timer messages that's
                    // why we don't need to call TranslateMessage or
                    // TranslateAccelerator, because keybord or
                    // mouse messages are for the FolderPicker which
                    // is in the foreground and should not arrive here
                    MSG msg;
                    while ( PeekMessageA( &msg, NULL, 0, 0, PM_REMOVE ) )
                        DispatchMessageA(&msg);
                }
                break;

            // should not happen
            default:
                OSL_ASSERT( sal_False );
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

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

void SAL_CALL CMtaFolderPicker::setDisplayDirectory( const OUString& aDirectory )
{
    m_displayDir = aDirectory;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

OUString SAL_CALL CMtaFolderPicker::getDisplayDirectory( )
{
    return m_displayDir;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

OUString SAL_CALL CMtaFolderPicker::getDirectory( )
{
    return m_SelectedDir;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

void SAL_CALL CMtaFolderPicker::setDescription( const rtl::OUString& aDescription )
{
    m_Description = aDescription;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

void SAL_CALL CMtaFolderPicker::setTitle( const OUString& aTitle )
{
    m_dialogTitle = aTitle;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

OUString SAL_CALL CMtaFolderPicker::getTitle( )
{
    return m_dialogTitle;
}

//-----------------------------------------------------
// XCancellable
//-----------------------------------------------------

void SAL_CALL CMtaFolderPicker::cancel( )
{
    if ( IsWindow( m_hwnd ) )
    {
        // simulate a mouse click to the
        // cancel button
        PostMessageA(
            m_hwnd,
            WM_COMMAND,
            MAKEWPARAM( IDCANCEL, BN_CLICKED ),
            (LPARAM)GetDlgItem( m_hwnd, IDCANCEL ) );
    }
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

sal_Bool SAL_CALL CMtaFolderPicker::onBrowseForFolder( )
{
    sal_Bool     bRet;
    LPITEMIDLIST lpiid;

    // pre SHBrowseFroFolder

    m_bi.pidlRoot       = 0;
    m_bi.pszDisplayName = m_pathBuff;

    if ( m_Description.getLength( ) )
        m_bi.lpszTitle = m_Description.getStr( );

    lpiid = SHBrowseForFolderW( &m_bi );
    bRet = ( NULL != lpiid );

    // post SHBrowseForFolder

    m_SelectedDir = getPathFromItemIdList( lpiid );
    releaseItemIdList( lpiid );

    return bRet;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

void SAL_CALL CMtaFolderPicker::releaseItemIdList( LPITEMIDLIST lpItemIdList )
{
    IMallocPtr pIMalloc;
    SHGetMalloc( &pIMalloc );

    try
    {
        pIMalloc->Free( lpItemIdList );
        lpItemIdList = NULL;
    }
    catch( _com_error& )
    {
        OSL_ASSERT( sal_False );
    }
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

LPITEMIDLIST SAL_CALL CMtaFolderPicker::getItemIdListFromPath( const rtl::OUString& aDirectory )
{
    // parameter checking
    if ( !aDirectory.getLength( ) )
        return NULL;

    IMallocPtr pIMalloc;
    SHGetMalloc( &pIMalloc );

    LPITEMIDLIST lpItemIdList = NULL;

    try
    {
        lpItemIdList = static_cast< LPITEMIDLIST >(
            pIMalloc->Alloc( sizeof( ITEMIDLIST ) ) );

        if ( lpItemIdList )
        {
            IShellFolderPtr pIShellFolder;
            SHGetDesktopFolder( &pIShellFolder );

            pIShellFolder->ParseDisplayName(
                NULL,
                NULL,
                const_cast< sal_Unicode* >( aDirectory.getStr( ) ),
                NULL,
                &lpItemIdList,
                NULL );
        }
    }
    catch( _com_error& )
    {
        if ( pIMalloc )
            pIMalloc->Free( lpItemIdList );

        lpItemIdList = NULL;
    }

    return lpItemIdList;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

OUString SAL_CALL CMtaFolderPicker::getPathFromItemIdList( LPCITEMIDLIST lpItemIdList )
{
    OUString path;

    if ( lpItemIdList )
    {
        sal_Bool bRet = SHGetPathFromIDListW( lpItemIdList, m_pathBuff );
        if ( bRet )
            path = m_pathBuff.get( );
    }

    return path;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

void SAL_CALL CMtaFolderPicker::enableOk( sal_Bool bEnable )
{
    OSL_ASSERT( IsWindow( m_hwnd ) );

    SendMessageW(
        m_hwnd,
        BFFM_ENABLEOK,
        static_cast< WPARAM >( 0 ),
        static_cast< LPARAM >( bEnable ) );
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

void SAL_CALL CMtaFolderPicker::setSelection( const rtl::OUString& aDirectory )
{
    OSL_ASSERT( IsWindow( m_hwnd ) );

#pragma message( "#######################################" )
#pragma message( "SendMessageW wrapper has to be extended" )
#pragma message( "#######################################" )

    SendMessageW(
        m_hwnd,
        BFFM_SETSELECTIONW,
        static_cast< WPARAM >( sal_True ),
        reinterpret_cast< LPARAM >( aDirectory.getStr( ) ) );
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

void SAL_CALL CMtaFolderPicker::setStatusText( const rtl::OUString& aStatusText )
{
    OSL_ASSERT( IsWindow( m_hwnd ) );

    SendMessageW(
        m_hwnd,
        BFFM_SETSTATUSTEXTW,
        static_cast< WPARAM >( 0 ),
        reinterpret_cast< LPARAM >( aStatusText.getStr( ) ) );
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

void SAL_CALL CMtaFolderPicker::onInitialized( )
{
    LPITEMIDLIST lpiidDisplayDir = getItemIdListFromPath( m_displayDir );

    if ( lpiidDisplayDir )
    {
        SendMessageA(
            m_hwnd,
            BFFM_SETSELECTION,
            (WPARAM)FALSE,
            (LPARAM) lpiidDisplayDir );

        releaseItemIdList( lpiidDisplayDir );
    }
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

void SAL_CALL CMtaFolderPicker::onSelChanged( const OUString& aNewPath )
{
    // to be overwritten by subclasses
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

sal_uInt32 SAL_CALL CMtaFolderPicker::onValidateFailed( sal_Unicode* lpInvalidPath )
{
    // to be overwritten by subclasses
    return 1;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

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
            SetWindowTextW( hwnd, pImpl->m_dialogTitle );
        break;

        case BFFM_SELCHANGED:
            pImpl->m_hwnd = hwnd;
            pImpl->onSelChanged(
                pImpl->getPathFromItemIdList(
                    reinterpret_cast< LPITEMIDLIST >( lParam ) ) );
        break;

        case BFFM_VALIDATEFAILEDW:
            nRC = pImpl->onValidateFailed( reinterpret_cast< sal_Unicode* >( lParam ) );
            break;

        default:
            OSL_ASSERT( sal_False );
    }

    return nRC;
}

//--------------------------------------------------------------------
// the window proc
//--------------------------------------------------------------------

LRESULT CALLBACK CMtaFolderPicker::StaWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LRESULT           lResult = 0;
    CMtaFolderPicker* pImpl   = NULL;

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
                LPCREATESTRUCT lpcs =
                    reinterpret_cast< LPCREATESTRUCT >( lParam );

                OSL_ASSERT( lpcs->lpCreateParams );

                // connect the instance handle to the window
                SetPropA( hWnd, CURRENT_INSTANCE, lpcs->lpCreateParams );
            }
            break;

        case WM_NCDESTROY:
                // RemoveProp returns the saved value on success
                pImpl = reinterpret_cast< CMtaFolderPicker* >(
                    RemovePropA( hWnd, CURRENT_INSTANCE ) );

                OSL_ASSERT( pImpl && !IsBadReadPtr( pImpl, sizeof( CMtaFolderPicker ) ) );
            break;

        case MSG_BROWSEFORFOLDER:
            {
                RequestContext* aReqCtx = reinterpret_cast< RequestContext* >( lParam );
                OSL_ASSERT( aReqCtx );

                pImpl = reinterpret_cast< CMtaFolderPicker* >(
                    GetPropA( hWnd, CURRENT_INSTANCE ) );

                OSL_ASSERT( pImpl && !IsBadReadPtr( pImpl, sizeof( CMtaFolderPicker ) ) );

                aReqCtx->bRet = pImpl->onBrowseForFolder( );
                SetEvent( aReqCtx->hEvent );
            }
            break;

        case MSG_SHUTDOWN:
            pImpl = reinterpret_cast< CMtaFolderPicker* >(
                GetPropA( hWnd, CURRENT_INSTANCE ) );

            OSL_ASSERT( pImpl && !IsBadReadPtr( pImpl, sizeof( CMtaFolderPicker ) ) );

            DestroyWindow( pImpl->m_hwndStaRequestWnd );
            break;

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

sal_Bool SAL_CALL CMtaFolderPicker::createStaRequestWindow( )
{
    sal_Bool bIsWnd = sal_False;

    if ( RegisterStaRequestWindowClass( ) )
    {
        m_hwndStaRequestWnd = CreateWindowA(
            g_szWndClsName, NULL,
            0, 0, 0, 0, 0,
            NULL, NULL, m_hInstance,
            (LPVOID)this // provide the instance of the class
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

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

unsigned int CMtaFolderPicker::run( )
{
    OSL_ASSERT( m_hEvtThrdReady );

    // setup an sta environment
    HRESULT hr = CoInitialize( NULL );

    // if we can't setup an sta environment
    // we stop here and return
    if ( FAILED( hr ) )
    {
        OSL_ENSURE( sal_False, "CoInitialize failed" );
        return -1;
    }

    unsigned int nRet;

    if ( createStaRequestWindow( ) )
    {
        SetEvent( m_hEvtThrdReady );

        // pumping messages
        MSG msg;
        while( GetMessageA( &msg, NULL, 0, 0 ) )
            DispatchMessageA( &msg );

        nRet = 0;
    }
    else
    {
        OSL_ENSURE( sal_False, "failed to create sta thread" );
        nRet = -1;
    }

    // shutdown sta environment
    CoUninitialize( );

    return nRet;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

unsigned int WINAPI CMtaFolderPicker::StaThreadProc( LPVOID pParam )
{
    CMtaFolderPicker* pInst =
        reinterpret_cast<CMtaFolderPicker*>( pParam );

    OSL_ASSERT( pInst );

    return pInst->run( );
}

//---------------------------------------------------
//
//---------------------------------------------------

ATOM SAL_CALL CMtaFolderPicker::RegisterStaRequestWindowClass( )
{
    osl::MutexGuard aGuard( s_Mutex );

    if ( 0 == s_ClassAtom )
    {
        WNDCLASSEXA  wcex;

        ZeroMemory( &wcex, sizeof( WNDCLASSEXA ) );

        wcex.cbSize         = sizeof(WNDCLASSEXA);
        wcex.style          = 0;
        wcex.lpfnWndProc    = static_cast< WNDPROC >( CMtaFolderPicker::StaWndProc );
        wcex.cbClsExtra     = 0;
        wcex.cbWndExtra     = 0;
        wcex.hInstance      = m_hInstance;
        wcex.hIcon          = NULL;
        wcex.hCursor        = NULL;
        wcex.hbrBackground  = NULL;
        wcex.lpszMenuName   = NULL;
        wcex.lpszClassName  = g_szWndClsName;
        wcex.hIconSm        = NULL;

        s_ClassAtom = RegisterClassExA( &wcex );
        OSL_ASSERT( s_ClassAtom );
    }

    // increment the register class counter
    // so that we keep track of the number
    // of class registrations
    if ( 0 != s_ClassAtom )
        s_StaRequestWndRegisterCount++;

    return s_ClassAtom;
}

//---------------------------------------------------
//
//---------------------------------------------------

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
        UnregisterClass(
            (LPCTSTR)MAKELONG( s_ClassAtom, 0 ), m_hInstance );

        s_ClassAtom = 0;
    }
}
