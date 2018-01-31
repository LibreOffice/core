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

#ifndef INCLUDED_FPICKER_SOURCE_WIN32_FOLDERPICKER_MTAFOP_HXX
#define INCLUDED_FPICKER_SOURCE_WIN32_FOLDERPICKER_MTAFOP_HXX

#include <sal/config.h>

#include <sal/log.hxx>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <utility>
#include <objidl.h>
#include <shlobj.h>
#include "../misc/WinImplHelper.hxx"

// the Mta-Ole clipboard class is for internal use only!
// only one instance of this class should be created, the
// user has to ensure this!
// the class is not thread-safe because it will be used
// only from within the clipboard service and the methods
// of the clipboard service are already synchronized


class CMtaFolderPicker
{
public:
    explicit CMtaFolderPicker( sal_uInt32 Flags );
    virtual ~CMtaFolderPicker( );

    // shell functions
    bool browseForFolder( );

    virtual void setDisplayDirectory( const OUString& aDirectory );
    virtual OUString getDisplayDirectory( );
    virtual OUString getDirectory( );

    virtual void setDescription( const OUString& aDescription );

    virtual void setTitle( const OUString& aTitle );
    OUString getTitle( );


    // XCancellable


    virtual void cancel( );

protected:
    void enableOk( bool bEnable );
    void setSelection( const OUString& aDirectory );
    void setStatusText( const OUString& aStatusText );

    virtual void onInitialized( );
    virtual void onSelChanged( const OUString& aNewPath ) = 0;

private:
    static sal_uInt32 onValidateFailed();

    // helper functions
    static LPITEMIDLIST getItemIdListFromPath( const OUString& aDirectory );
    OUString getPathFromItemIdList( LPCITEMIDLIST lpItemIdList );
    static void releaseItemIdList( LPITEMIDLIST lpItemIdList );

    unsigned int run( );

    // create a hidden windows which serves as an request
    // target; so we guarantee synchronization
    bool createStaRequestWindow( );


    // message handler functions; remember these functions are called
    // from a different thread context!


    bool onBrowseForFolder( );

    static LRESULT CALLBACK StaWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static unsigned int WINAPI StaThreadProc( LPVOID pParam );

    static int CALLBACK FolderPickerCallback( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData );

protected:
    HWND m_hwnd;

private:
    ATOM RegisterStaRequestWindowClass( );
    void UnregisterStaRequestWindowClass( );

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
    sal_Unicode                 m_pathBuff[32000]; // max path length under Win2000
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
