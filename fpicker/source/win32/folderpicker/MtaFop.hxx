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
#include "../misc/WinImplHelper.hxx"


// a simple helper class used to provide a buffer for different
// Win32 file and directory functions


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


// the Mta-Ole clipboard class is for internal use only!
// only one instance of this class should be created, the
// user has to ensure this!
// the class is not thread-safe because it will be used
// only from within the clipboard service and the methods
// of the clipboard service are already synchronized


class CMtaFolderPicker
{
public:
    CMtaFolderPicker( sal_uInt32 Flags );
    virtual ~CMtaFolderPicker( );

    // shell functions
    sal_Bool SAL_CALL browseForFolder( );

    virtual void  SAL_CALL setDisplayDirectory( const OUString& aDirectory );
    virtual OUString  SAL_CALL getDisplayDirectory( );
    virtual OUString SAL_CALL getDirectory( );

    virtual void SAL_CALL setDescription( const OUString& aDescription );

    virtual void SAL_CALL setTitle( const OUString& aTitle );
    OUString  SAL_CALL getTitle( );


    // XCancellable


    virtual void SAL_CALL cancel( );

protected:
    void SAL_CALL enableOk( sal_Bool bEnable );
    void SAL_CALL setSelection( const OUString& aDirectory );
    void SAL_CALL setStatusText( const OUString& aStatusText );

    virtual void SAL_CALL onInitialized( );
    virtual void SAL_CALL onSelChanged( const OUString& aNewPath ) = 0;

private:
    sal_uInt32 onValidateFailed();

    // helper functions
    LPITEMIDLIST  SAL_CALL getItemIdListFromPath( const OUString& aDirectory );
    OUString SAL_CALL getPathFromItemIdList( LPCITEMIDLIST lpItemIdList );
    void SAL_CALL releaseItemIdList( LPITEMIDLIST lpItemIdList );

    unsigned int run( );

    // create a hidden windows which serves as an request
    // target; so we guarantee synchronization
    sal_Bool SAL_CALL createStaRequestWindow( );


    // message handler functions; remember these functions are called
    // from a different thread context!


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
    OUString               m_dialogTitle;
    OUString               m_Description;
    OUString               m_displayDir;
    OUString               m_SelectedDir;
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
