/*************************************************************************
 *
 *  $RCSfile: MtaFop.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2001-06-28 11:15:52 $
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

#include "MtaFop.hxx"
#include <wchar.h>
#include <process.h>
#include <comdef.h>

#include <systools/win32/shell9x.h>

#ifndef _RESOURCEPROVIDER_HXX_
#include "..\misc\resourceprovider.hxx"
#endif

//----------------------------------------------------------------
//  namespace directives
//----------------------------------------------------------------

using rtl::OUString;

//----------------------------------------------------------------
//  defines
//----------------------------------------------------------------

namespace
{
    char FOLDERPICKER_SRV_DLL_NAME[] = "fop.dll";
    char g_szWndClsName[]            = "MtaOleReqWnd###";
}

//--------------------------------------------------------
// messages constants
//--------------------------------------------------------

const sal_uInt32 MSG_BROWSEFORFOLDER = WM_USER + 1;
const sal_uInt32 MSG_SHUTDOWN        = WM_USER + 2;

const sal_uInt32 MAX_WAITTIME        = 10000; // msec

const sal_Bool MANUAL_RESET     = sal_True;
const sal_Bool AUTO_RESET       = sal_False;
const sal_Bool INIT_NONSIGNALED = sal_False;

//----------------------------------------------------------------
//  static member initialization
//----------------------------------------------------------------

CMtaFolderPicker* CMtaFolderPicker::s_mtaFolderPickerInst = NULL;

//--------------------------------------------------------------------
// ctor
//--------------------------------------------------------------------

CMtaFolderPicker::CMtaFolderPicker( sal_uInt32 Flags ) :
    m_hStaThread( NULL ),
    m_uStaThreadId( 0 ),
    m_hEvtThrdReady( NULL ),
    m_hwndMtaFolderPickerReqWnd( NULL ),
    m_MtaFolderPickerReqWndClassAtom( 0 ),
    m_hwndFolderPicker( NULL )
{
    ZeroMemory( &m_bi, sizeof( m_bi ) );

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

    m_bi.lpfn    = CMtaFolderPicker::folderPickerCallback;
    m_bi.lParam  = reinterpret_cast< LPARAM >( this );

    //---------------------------------------
    // read the default strings for title and
    // description from a resource file

    CResourceProvider ResProvider;

    m_dialogTitle = ResProvider.getResString( 500 );
    m_Description = ResProvider.getResString( 501 );

    // signals that the thread was
    // successfully set up
    m_hEvtThrdReady  = CreateEventA(
        0,
        MANUAL_RESET,
        INIT_NONSIGNALED,
        NULL );
    OSL_ASSERT( NULL != m_hEvtThrdReady );

    s_mtaFolderPickerInst = this;

    // setup the sta thread
    m_hStaThread = (HANDLE)_beginthreadex(
        NULL, 0, CMtaFolderPicker::oleThreadProc, this, 0, &m_uStaThreadId );
    OSL_ASSERT( NULL != m_hStaThread );
}

//--------------------------------------------------------------------
// dtor
//--------------------------------------------------------------------

CMtaFolderPicker::~CMtaFolderPicker( )
{
    // block calling threads out
    if ( NULL != m_hEvtThrdReady )
        ResetEvent( m_hEvtThrdReady );

    // end the thread
    // because DestroyWindow can only be called
    // from within the thread that created the window
    SendMessageA(
        m_hwndMtaFolderPickerReqWnd,
        MSG_SHUTDOWN,
        static_cast< WPARAM >( 0 ),
        static_cast< LPARAM >( 0 ) );

    // wait for thread shutdown
    sal_uInt32 dwResult =
        WaitForSingleObject( m_hStaThread, MAX_WAITTIME );
    OSL_ENSURE( dwResult == WAIT_OBJECT_0, "Sta-Thread could not terminate" );

    // terminate the thread if it doesn't shutdown itself
    if ( WAIT_OBJECT_0 != dwResult )
        TerminateThread( m_hStaThread, 0 );

    if ( NULL != m_hStaThread )
        CloseHandle( m_hStaThread );

    if ( NULL != m_hEvtThrdReady )
        CloseHandle( m_hEvtThrdReady );

    if ( m_MtaFolderPickerReqWndClassAtom )
        UnregisterClassA( g_szWndClsName, NULL );

    s_mtaFolderPickerInst = NULL;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

sal_Bool CMtaFolderPicker::browseForFolder( )
{
    /*
    if ( !WaitForThreadReady( ) )
        return sal_False;

    // marshall request into the sta thread
    return static_cast< sal_Bool >( SendMessageA(
        m_hwndMtaFolderPickerReqWnd,
        MSG_BROWSEFORFOLDER,
        0,
        0 ) );
    */
    return onBrowseForFolder( );
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

    m_SelectedDir = getPathFromIdList( lpiid );
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

OUString SAL_CALL CMtaFolderPicker::getPathFromIdList( LPCITEMIDLIST lpItemIdList )
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

    SendMessageA(
        m_hwnd,
        BFFM_SETSELECTION,
        (WPARAM)FALSE,
        (LPARAM) lpiidDisplayDir );

    releaseItemIdList( lpiidDisplayDir );
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

int CALLBACK CMtaFolderPicker::folderPickerCallback( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
    CMtaFolderPicker* pImpl = reinterpret_cast< CMtaFolderPicker* >( lpData );
    OSL_ASSERT( pImpl );

    pImpl->m_hwnd = hwnd;

    int nRC = 0;

    switch( uMsg )
    {
        case BFFM_INITIALIZED:
            pImpl->onInitialized( );
            SetWindowTextW( hwnd, pImpl->m_dialogTitle );
        break;

        case BFFM_SELCHANGED:
            pImpl->onSelChanged(
                pImpl->getPathFromIdList(
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

LRESULT CALLBACK CMtaFolderPicker::mtaFolderPickerReqWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LRESULT lResult = 0;

    __try
    {
        // get a connection to the class-instance via the static member
        CMtaFolderPicker* pImpl = CMtaFolderPicker::s_mtaFolderPickerInst;
        OSL_ASSERT( NULL != pImpl );

        switch( uMsg )
        {
        case MSG_BROWSEFORFOLDER:
            lResult = pImpl->onBrowseForFolder( );
            break;

        case MSG_SHUTDOWN:
            DestroyWindow( pImpl->m_hwndMtaFolderPickerReqWnd );
            break;

        // under windows 95/98 the creation of the
        // hidden target request window fails if
        // we don't handle this message ourself
        // because the DefWindowProc returns 0 as
        // a result of handling WM_NCCREATE what
        // leads to a failure of CreateWindow[Ex]!!!
        case WM_NCCREATE:
            lResult = TRUE;
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            lResult = DefWindowProc( hWnd, uMsg, wParam, lParam );
            break;
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        OSL_ENSURE( sal_False, "Kernel exception in window-proc caught!" );
    }

    return lResult;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

void CMtaFolderPicker::createMtaFolderPickerReqWnd( )
{
    WNDCLASSEXA  wcex;

    HINSTANCE hInst = GetModuleHandleA( FOLDERPICKER_SRV_DLL_NAME );
    OSL_ENSURE( NULL != hInst, "The name of the FolderPicker service dll must have changed" );

    ZeroMemory( &wcex, sizeof( WNDCLASSEXA ) );

    wcex.cbSize         = sizeof(WNDCLASSEXA);
    wcex.style          = 0;
    wcex.lpfnWndProc    = static_cast< WNDPROC >( CMtaFolderPicker::mtaFolderPickerReqWndProc );
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInst;
    wcex.hIcon          = NULL;
    wcex.hCursor        = NULL;
    wcex.hbrBackground  = NULL;
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = g_szWndClsName;
    wcex.hIconSm        = NULL;

    m_MtaFolderPickerReqWndClassAtom = RegisterClassExA( &wcex );

    if ( 0 != m_MtaFolderPickerReqWndClassAtom )
        m_hwndMtaFolderPickerReqWnd = CreateWindowA(
            g_szWndClsName, NULL, 0, 0, 0, 0, 0, NULL, NULL, hInst, NULL );
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

unsigned int CMtaFolderPicker::run( )
{
    HRESULT hr = CoInitialize( NULL );
    OSL_ASSERT( SUCCEEDED( hr ) );

    createMtaFolderPickerReqWnd( );

    unsigned int nRet;

    if ( IsWindow( m_hwndMtaFolderPickerReqWnd ) )
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
        nRet = -1;

    CoUninitialize( );

    return nRet;
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

unsigned int WINAPI CMtaFolderPicker::oleThreadProc( LPVOID pParam )
{
    CMtaFolderPicker* pInst =
        reinterpret_cast<CMtaFolderPicker*>( pParam );
    OSL_ASSERT( NULL != pInst );

    return pInst->run( );
}

//--------------------------------------------------------------------
//
//--------------------------------------------------------------------

inline
sal_Bool CMtaFolderPicker::WaitForThreadReady( ) const
{
    sal_Bool bRet = sal_False;

    if ( NULL != m_hEvtThrdReady )
    {
        DWORD dwResult = WaitForSingleObject( m_hEvtThrdReady, MAX_WAITTIME );
        bRet = ( dwResult == WAIT_OBJECT_0 );
    }

    return bRet;
}

