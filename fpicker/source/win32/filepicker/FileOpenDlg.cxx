/*************************************************************************
 *
 *  $RCSfile: FileOpenDlg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tra $ $Date: 2001-08-10 12:12:37 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _WINIMPLHELPER_HXX_
#include "..\misc\WinImplHelper.hxx"
#endif

#ifndef _FILEOPENDLG_HXX_
#include "FileOpenDlg.hxx"
#endif

#include <systools/win32/comdlg9x.h>
#include <systools/win32/user9x.h>

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using rtl::OUString;

//------------------------------------------------------------------------
// constants
//------------------------------------------------------------------------

namespace /* private */
{
    // we choose such large buffers because the size of
    // an single line edit field can be up to 32k; if
    // a user has a multi selection FilePicker and selects
    // a lot of files in a large directory we may reach this
    // limit and don't want to get out of memory;
    // another much more elegant way would be to subclass the
    // FileOpen dialog and overload the BM_CLICK event of the
    // OK button so that we determine the size of the text
    // currently in the edit field and resize our buffer
    // appropriately - in the future we will do this
    const size_t MAX_FILENAME_BUFF_SIZE  = 32000;
    const size_t MAX_FILETITLE_BUFF_SIZE = 32000;
    const size_t MAX_FILTER_BUFF_SIZE    = 4096;

    const char* CURRENT_INSTANCE = "CurrInst";
};

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFileOpenDialog::CFileOpenDialog(
    sal_Bool bFileOpenDialog,
    sal_uInt32 dwFlags,
    sal_uInt32 dwTemplateId,
    HINSTANCE hInstance ) :
    m_filterBuffer( MAX_FILTER_BUFF_SIZE, sal_True ),
    m_fileNameBuffer( MAX_FILENAME_BUFF_SIZE ),
    m_fileTitleBuffer( MAX_FILETITLE_BUFF_SIZE ),
    m_helperBuffer( MAX_FILENAME_BUFF_SIZE ),
    m_hwndFileOpenDlg( 0 ),
    m_hwndFileOpenDlgChild( 0 ),
    m_bFileOpenDialog( bFileOpenDialog )
{
    // initialize the OPENFILENAME struct
    if ( IsWin2000( ) )
    {
        ZeroMemory( &m_ofn, sizeof( m_ofn ) );
        m_ofn.lStructSize = sizeof( m_ofn );
    }
    else // OSVER < Win2000
    {
        // the size of the OPENFILENAME structure is different
        // under windows < win2000
        ZeroMemory( &m_ofn, _OPENFILENAME_SIZE_VERSION_400W );
        m_ofn.lStructSize = _OPENFILENAME_SIZE_VERSION_400W;
    }

    m_ofn.Flags |= dwFlags |
                   OFN_EXPLORER |
                   OFN_ENABLEHOOK |
                   OFN_HIDEREADONLY |
                   OFN_PATHMUSTEXIST |
                   OFN_FILEMUSTEXIST |
                   OFN_OVERWRITEPROMPT;

    // it is a little hack but how else could
    // we get a parent window (using a vcl window?)
    m_ofn.hwndOwner = GetForegroundWindow( );

    // we don't use custom filters
    m_ofn.lpstrCustomFilter = 0;

    m_ofn.lpstrFile = m_fileNameBuffer;
    m_ofn.nMaxFile  = m_fileNameBuffer.size( );

    m_ofn.lpstrFileTitle = m_fileTitleBuffer;
    m_ofn.nMaxFileTitle  = m_fileTitleBuffer.size( );

    m_ofn.lpfnHook = CFileOpenDialog::ofnHookProc;

    // set a custom template
    if ( dwTemplateId )
    {
        OSL_ASSERT( hInstance );

        m_ofn.Flags |= OFN_ENABLETEMPLATE;
        m_ofn.lpTemplateName = MAKEINTRESOURCEW( dwTemplateId );
        m_ofn.hInstance = hInstance;
    }

    // set a pointer to myself as ofn parameter
    m_ofn.lCustData = reinterpret_cast< long > ( this );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFileOpenDialog::~CFileOpenDialog( )
{
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFileOpenDialog::setTitle( const OUString& aTitle )
{
    m_dialogTitle = aTitle.getStr( );
    m_ofn.lpstrTitle = m_dialogTitle.getStr( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void CFileOpenDialog::setFilter( const OUString& aFilter )
{
    m_filterBuffer.resize( aFilter.getLength( ) + 1 );
    m_filterBuffer.fill( aFilter.getStr( ), aFilter.getLength( ) );
    m_ofn.lpstrFilter = m_filterBuffer;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool CFileOpenDialog::setFilterIndex( sal_uInt32 aIndex )
{
    OSL_ASSERT( aIndex > 0 );
    m_ofn.nFilterIndex = aIndex;
    return sal_True;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_uInt32 CFileOpenDialog::getSelectedFilterIndex( ) const
{
    return m_ofn.nFilterIndex;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFileOpenDialog::setDefaultName( const OUString& aName )
{
    m_fileNameBuffer.empty( );
    m_fileNameBuffer.fill( aName.getStr( ), aName.getLength( ) );
    m_ofn.lpstrFile = m_fileNameBuffer;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFileOpenDialog::setDisplayDirectory( const OUString& aDirectory )
{
    m_displayDirectory = aDirectory;
    m_ofn.lpstrInitialDir = m_displayDirectory;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

rtl::OUString SAL_CALL CFileOpenDialog::getLastDisplayDirectory( ) const
{
    return m_displayDirectory;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

rtl::OUString SAL_CALL CFileOpenDialog::getFullFileName( ) const
{
    return OUString( &m_fileNameBuffer, _wcslenex( &m_fileNameBuffer ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

rtl::OUString SAL_CALL CFileOpenDialog::getFileName( ) const
{
    return OUString( &m_fileTitleBuffer, wcslen( &m_fileTitleBuffer ) + 1 );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

rtl::OUString CFileOpenDialog::getFileExtension( )
{
    OUString fileExt;

    if ( m_ofn.nFileExtension )
        fileExt = OUString( m_fileNameBuffer + m_ofn.nFileExtension,
            wcslen( m_fileNameBuffer + m_ofn.nFileExtension ) );

    return fileExt;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void CFileOpenDialog::setDefaultFileExtension( const OUString& aExtension )
{
    m_defaultExtension = aExtension;
    m_ofn.lpstrDefExt = m_defaultExtension.getStr( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFileOpenDialog::setMultiSelectionMode( sal_Bool bMode )
{
    if ( bMode )
        m_ofn.Flags |= OFN_ALLOWMULTISELECT;
    else if ( m_ofn.Flags & OFN_ALLOWMULTISELECT )
        m_ofn.Flags ^= OFN_ALLOWMULTISELECT;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CFileOpenDialog::getMultiSelectionMode( ) const
{
    return ( m_ofn.Flags & OFN_ALLOWMULTISELECT );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool CFileOpenDialog::setControlLabel( sal_Int16 ElementID, const OUString& aLabel )
{
    return sal_True;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFileOpenDialog::enableControl( sal_Int16 ElementID, sal_Bool bEnabled )
{

}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool CFileOpenDialog::showControl( sal_Int16 ElementID, sal_Bool bShow )
{
    return sal_True;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFileOpenDialog::POSITION SAL_CALL CFileOpenDialog::beginEnumFileNames( )
{
    return 0;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CFileOpenDialog::getNextFileName(
    POSITION& pos, OUString& aNextFileName )
{
    return sal_False;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Int16 SAL_CALL CFileOpenDialog::doModal( )
{
    sal_Int16 nRC = -1;

    // pre-processing
    if ( preModal( ) )
    {
        sal_Bool bRet;
        if ( m_bFileOpenDialog )
            bRet = ::GetOpenFileNameW( reinterpret_cast< LPOPENFILENAMEW >( &m_ofn ) );
        else
            bRet = ::GetSaveFileNameW( reinterpret_cast< LPOPENFILENAMEW >( &m_ofn ) );

        nRC = 1;
        if ( !bRet )
        {
#ifdef _DEBUG
            sal_uInt32 nError = CommDlgExtendedError( );
            nRC = (0 == nError) ? 0 : -1;
#else
            nRC = (0 == CommDlgExtendedError( )) ? 0 : -1;
#endif
        }

        // post-processing
        postModal( nRC );
    }

    return nRC;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_uInt32 SAL_CALL CFileOpenDialog::getLastDialogError( ) const
{
    return CommDlgExtendedError( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CFileOpenDialog::preModal( )
{
    return sal_True;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFileOpenDialog::postModal( sal_Int16 nDialogResult )
{
    OSL_ASSERT( (-1 <= nDialogResult) && (nDialogResult <= 1) );

    if ( m_ofn.lpstrFile )
    {
        OUString lastDir;
        sal_Unicode* pLastDirChar = m_ofn.lpstrFile + m_ofn.nFileOffset - 1;
        if ( m_ofn.nFileOffset && 0 == *pLastDirChar )
            lastDir = OUString( m_ofn.lpstrFile, m_ofn.nFileOffset - 1 );
        else if ( m_ofn.nFileOffset )
            lastDir = OUString( m_ofn.lpstrFile, m_ofn.nFileOffset );

        m_displayDirectory = lastDir;
    }
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CFileOpenDialog::getCurrentFilePath( ) const
{
    OSL_ASSERT( IsWindow( m_hwndFileOpenDlg ) );

    LPARAM nLen = SendMessageW(
        m_hwndFileOpenDlg,
        CDM_GETFILEPATH,
        m_helperBuffer.size( ),
        reinterpret_cast< LPARAM >( &m_helperBuffer ) );

    OUString filePath;

    if ( nLen > 0 )
        filePath = OUString( &m_helperBuffer, (nLen - 1) );

    return filePath;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CFileOpenDialog::getCurrentFolderPath( ) const
{
    OSL_ASSERT( IsWindow( m_hwndFileOpenDlg ) );

    LPARAM nLen = SendMessageW(
        m_hwndFileOpenDlg,
        CDM_GETFOLDERPATH,
        m_helperBuffer.size( ),
        reinterpret_cast< LPARAM >( &m_helperBuffer ) );

    OUString folderPath;

    if ( nLen > 0 )
        folderPath = OUString( &m_helperBuffer, (nLen -1) );

    return folderPath;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CFileOpenDialog::getCurrentFileName( ) const
{
    OSL_ASSERT( IsWindow( m_hwndFileOpenDlg ) );

    // this is an ugly hack because beause
    // CDM_GETSPEC and BFFM_SETSTATUSTEXT
    // message id are equal and we have only
    // one SendMessageW wrapper for Win95
    int MsgId = CDM_GETSPEC;

    OSVERSIONINFO   OSVerInfo;

    OSVerInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
    GetVersionEx( &OSVerInfo );

    // if windows 95/98
    if ( VER_PLATFORM_WIN32_WINDOWS == OSVerInfo.dwPlatformId )
        MsgId = CDM_GETSPEC + 100;

    LPARAM nLen = SendMessageW(
        m_hwndFileOpenDlg,
        MsgId,
        m_helperBuffer.size( ),
        reinterpret_cast< LPARAM >( &m_helperBuffer ) );

    OUString fileName;

    if ( nLen > 0 )
        fileName = OUString( &m_helperBuffer, (nLen - 1) );

    return fileName;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_uInt32 SAL_CALL CFileOpenDialog::onShareViolation( const OUString& aPathName)
{
    return 0;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_uInt32 SAL_CALL CFileOpenDialog::onFileOk()
{
    return 0;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFileOpenDialog::onSelChanged( HWND hwndListBox )
{
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFileOpenDialog::onHelp( )
{
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFileOpenDialog::onInitDone()
{
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFileOpenDialog::onFolderChanged()
{
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFileOpenDialog::onTypeChanged( sal_uInt32 nFilterIndex )
{
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFileOpenDialog::onInitDialog( HWND hwndDlg, HWND hwndChild )
{

}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_uInt32 SAL_CALL CFileOpenDialog::onCtrlCommand(
    HWND hwndChild, sal_uInt16 ctrlId, sal_uInt16 notifyCode )
{
    return 0;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_uInt32 SAL_CALL CFileOpenDialog::onWMNotify( HWND hwndChild, LPOFNOTIFYW lpOfNotify )
{
    switch( lpOfNotify->hdr.code )
    {
    case CDN_SHAREVIOLATION:
        return onShareViolation( lpOfNotify->pszFile );

    case CDN_FILEOK:
        return onFileOk( );

    case CDN_SELCHANGE:
        onSelChanged( lpOfNotify->hdr.hwndFrom );
        break;

    case CDN_HELP:
        onHelp( );
        break;

    case CDN_INITDONE:
        onInitDone( );
        break;

    case CDN_FOLDERCHANGE:
        onFolderChanged( );
        break;

    case CDN_TYPECHANGE:
        m_ofn.nFilterIndex = lpOfNotify->lpOFN->nFilterIndex;
        onTypeChanged( lpOfNotify->lpOFN->nFilterIndex );
        break;
    }

    return 0;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFileOpenDialog::handleInitDialog( HWND hwndDlg, HWND hwndChild )
{
    m_hwndFileOpenDlg      = hwndDlg;
    m_hwndFileOpenDlgChild = hwndChild;

    OSL_ASSERT( GetParent( hwndChild ) == hwndDlg );

    // calling virtual function which the
    // client can overload
    onInitDialog( hwndDlg, hwndChild );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

unsigned int CALLBACK CFileOpenDialog::ofnHookProc(
    HWND hChildDlg, unsigned int uiMsg, WPARAM wParam, LPARAM lParam )
{
    HWND hwndDlg = GetParent( hChildDlg );
    CFileOpenDialog* pImpl = NULL;

    switch( uiMsg )
    {
    case WM_INITDIALOG:
        {
            _OPENFILENAMEW* lpofn = reinterpret_cast< _OPENFILENAMEW* >( lParam );
            pImpl = reinterpret_cast< CFileOpenDialog* >( lpofn->lCustData );
            OSL_ASSERT( pImpl );

            // subclass the base dialog for WM_NCDESTROY processing
            pImpl->m_pfnBaseDlgProc =
                reinterpret_cast< DLGPROC >(
                    SetWindowLong( hwndDlg,
                    DWL_DLGPROC,
                    reinterpret_cast< DWORD >( CFileOpenDialog::BaseDlgProc ) ) );

            // connect the instance handle to the window
            SetPropA( hwndDlg, CURRENT_INSTANCE, pImpl );

            pImpl->handleInitDialog( hwndDlg, hChildDlg );
        }
        return 0;

    case WM_NOTIFY:
        {
            pImpl = getCurrentInstance( hwndDlg );
            return pImpl->onWMNotify(
                hChildDlg, reinterpret_cast< LPOFNOTIFYW >( lParam ) );
        }

    case WM_COMMAND:
        {
            pImpl = getCurrentInstance( hwndDlg );
            OSL_ASSERT( pImpl );

            return pImpl->onCtrlCommand(
                hChildDlg, LOWORD( wParam ), HIWORD( lParam ) );
        }
    }

    return 0;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

unsigned int CALLBACK CFileOpenDialog::BaseDlgProc(
    HWND hWnd, WORD wMessage, WPARAM wParam, LPARAM lParam )
{
    unsigned int lResult   = 0;
    CFileOpenDialog* pImpl = NULL;

    switch( wMessage )
    {
    case WM_NCDESTROY:

        // RemoveProp returns the saved value on success
        pImpl = reinterpret_cast< CFileOpenDialog* >(
            RemovePropA( hWnd, CURRENT_INSTANCE ) );
        OSL_ASSERT( pImpl );

        // fall through in order to call the
        // base dlg proc
    default:
        if ( !pImpl )
        {
            pImpl = getCurrentInstance( hWnd );
            OSL_ASSERT( pImpl );
        }

        // !!! we use CallWindowProcA
        lResult = CallWindowProcA(
            reinterpret_cast< WNDPROC >( pImpl->m_pfnBaseDlgProc ),
            hWnd, wMessage, wParam, lParam );

    break;

    } // switch

    return lResult;

}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFileOpenDialog* CFileOpenDialog::getCurrentInstance( HWND hwnd )
{
    OSL_ASSERT( IsWindow( hwnd ) );
    return reinterpret_cast< CFileOpenDialog* >(
        GetPropA( hwnd, CURRENT_INSTANCE ) );
}