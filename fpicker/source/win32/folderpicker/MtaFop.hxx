/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _MTAFOP_HXX_
#define _MTAFOP_HXX_

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>

#include <utility>
#ifdef __MINGW32__
#include <windows.h>
#endif
#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable: 4917)
#endif
#include <objidl.h>
#include <shlobj.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include "..\misc\WinImplHelper.hxx"

//----------------------------------------------------------------
// a simple helper class used to provide a buffer for different
// Win32 file and directory functions
//----------------------------------------------------------------

class CAutoPathBuff
{
public:
    CAutoPathBuff( size_t size = 0 )
    {
        if (0 == size)
            size = 32000; // max path length under Win2000

        pBuff = new sal_Unicode[size];

        OSL_POSTCOND(pBuff,"Could not allocate path buffer");
    }

    ~CAutoPathBuff( )
    {
        delete [] pBuff;
    }

    operator sal_Unicode*( )
    {
        OSL_PRECOND( pBuff, \
            "No path buffer allocated" );
        return pBuff;
    }

    sal_Unicode* get( )
    {
        OSL_PRECOND( pBuff, \
            "No path buffer allocated" );
        return pBuff;
    }

private:
    sal_Unicode* pBuff;
};

//--------------------------------------------------------
// the Mta-Ole clipboard class is for internal use only!
// only one instance of this class should be created, the
// user has to ensure this!
// the class is not thread-safe because it will be used
// only from within the clipboard service and the methods
// of the clipboard service are already synchronized
//--------------------------------------------------------

class CMtaFolderPicker
{
public:
    CMtaFolderPicker( sal_uInt32 Flags );
    virtual ~CMtaFolderPicker( );

    // shell functions
    sal_Bool SAL_CALL browseForFolder( );

    virtual void  SAL_CALL setDisplayDirectory( const rtl::OUString& aDirectory );
    virtual rtl::OUString  SAL_CALL getDisplayDirectory( );
    virtual rtl::OUString SAL_CALL getDirectory( );

    virtual void SAL_CALL setDescription( const rtl::OUString& aDescription );

    virtual void SAL_CALL setTitle( const rtl::OUString& aTitle );
    rtl::OUString  SAL_CALL getTitle( );

    //-----------------------------------------------------
    // XCancellable
    //-----------------------------------------------------

    virtual void SAL_CALL cancel( );

protected:
    void SAL_CALL enableOk( sal_Bool bEnable );
    void SAL_CALL setSelection( const rtl::OUString& aDirectory );
    void SAL_CALL setStatusText( const rtl::OUString& aStatusText );

    virtual void SAL_CALL onInitialized( );
    virtual void SAL_CALL onSelChanged( const rtl::OUString& aNewPath ) = 0;

private:
    sal_uInt32 onValidateFailed();

    // helper functions
    LPITEMIDLIST  SAL_CALL getItemIdListFromPath( const rtl::OUString& aDirectory );
    rtl::OUString SAL_CALL getPathFromItemIdList( LPCITEMIDLIST lpItemIdList );
    void SAL_CALL releaseItemIdList( LPITEMIDLIST lpItemIdList );

    unsigned int run( );

    // create a hidden windows which serves as an request
    // target; so we guarantee synchronization
    sal_Bool SAL_CALL createStaRequestWindow( );

    //---------------------------------------------------------------
    // message handler functions; remeber these functions are called
    // from a different thread context!
    //---------------------------------------------------------------

    sal_Bool SAL_CALL onBrowseForFolder( );

    static LRESULT CALLBACK StaWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static unsigned int WINAPI StaThreadProc( LPVOID pParam );

    static int CALLBACK FolderPickerCallback( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData );

protected:
    HWND m_hwnd;

private:
    ATOM SAL_CALL RegisterStaRequestWindowClass( );
    void SAL_CALL UnregisterStaRequestWindowClass( );

private:
    HANDLE                      m_hStaThread;
    unsigned                    m_uStaThreadId;
    HANDLE                      m_hEvtThrdReady;
    HWND                        m_hwndStaRequestWnd;
    rtl::OUString               m_dialogTitle;
    rtl::OUString               m_Description;
    rtl::OUString               m_displayDir;
    rtl::OUString               m_SelectedDir;
    BROWSEINFOW                 m_bi;
    CAutoPathBuff               m_pathBuff;
    HINSTANCE                   m_hInstance;

    // the request window class has to be registered only
    // once per process, so multiple instance of this class
    // share the registered window class
    static ATOM       s_ClassAtom;
    static osl::Mutex s_Mutex;
    static sal_Int32  s_StaRequestWndRegisterCount;

// prevent copy and assignment
private:
    CMtaFolderPicker( const CMtaFolderPicker& );
    CMtaFolderPicker& operator=( const CMtaFolderPicker& );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
