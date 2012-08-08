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

#ifndef _FILEOPENDLG_HXX_
#define _FILEOPENDLG_HXX_

#include <sal/types.h>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include "platform_xp.h"
#include "getfilenamewrapper.hxx"

// because we don't want to import the new W2k platform skd
// into our build environment if have stolen the definition
// for the new OPENFILENAME structure from the new headers

#ifndef _CDSIZEOF_STRUCT
#define _CDSIZEOF_STRUCT(structname, member)  (((int)((LPBYTE)(&((structname*)0)->member) - ((LPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))
#endif

typedef struct _tagOFNA {
   DWORD        lStructSize;
   HWND         hwndOwner;
   HINSTANCE    hInstance;
   LPCSTR       lpstrFilter;
   LPSTR        lpstrCustomFilter;
   DWORD        nMaxCustFilter;
   DWORD        nFilterIndex;
   LPSTR        lpstrFile;
   DWORD        nMaxFile;
   LPSTR        lpstrFileTitle;
   DWORD        nMaxFileTitle;
   LPCSTR       lpstrInitialDir;
   LPCSTR       lpstrTitle;
   DWORD        Flags;
   WORD         nFileOffset;
   WORD         nFileExtension;
   LPCSTR       lpstrDefExt;
   LPARAM       lCustData;
   LPOFNHOOKPROC lpfnHook;
   LPCSTR       lpTemplateName;
#ifdef _MAC
   LPEDITMENU   lpEditInfo;
   LPCSTR       lpstrPrompt;
#endif
#if (_WIN32_WINNT >= 0x0500)
   void *       pvReserved;
   DWORD        dwReserved;
   DWORD        FlagsEx;
#endif // (_WIN32_WINNT >= 0x0500)
} _OPENFILENAMEA, *_LPOPENFILENAMEA;

typedef struct _tagOFNW {
   DWORD        lStructSize;
   HWND         hwndOwner;
   HINSTANCE    hInstance;
   LPCWSTR      lpstrFilter;
   LPWSTR       lpstrCustomFilter;
   DWORD        nMaxCustFilter;
   DWORD        nFilterIndex;
   LPWSTR       lpstrFile;
   DWORD        nMaxFile;
   LPWSTR       lpstrFileTitle;
   DWORD        nMaxFileTitle;
   LPCWSTR      lpstrInitialDir;
   LPCWSTR      lpstrTitle;
   DWORD        Flags;
   WORD         nFileOffset;
   WORD         nFileExtension;
   LPCWSTR      lpstrDefExt;
   LPARAM       lCustData;
   LPOFNHOOKPROC lpfnHook;
   LPCWSTR      lpTemplateName;
#if (_WIN32_WINNT >= 0x0500)
   void *       pvReserved;
   DWORD        dwReserved;
   DWORD        FlagsEx;
#endif // (_WIN32_WINNT >= 0x0500)
} _OPENFILENAMEW, *_LPOPENFILENAMEW;

#ifdef UNICODE
typedef _OPENFILENAMEW _OPENFILENAME;
typedef _LPOPENFILENAMEW _LPOPENFILENAME;
#else
typedef _OPENFILENAMEA _OPENFILENAME;
typedef _LPOPENFILENAMEA _LPOPENFILENAME;
#endif // UNICODE

#if (_WIN32_WINNT >= 0x0500)
    #define _OPENFILENAME_SIZE_VERSION_400A  _CDSIZEOF_STRUCT(_OPENFILENAMEA,lpTemplateName)
    #define _OPENFILENAME_SIZE_VERSION_400W  _CDSIZEOF_STRUCT(_OPENFILENAMEW,lpTemplateName)
    #ifdef UNICODE
        #define _OPENFILENAME_SIZE_VERSION_400  _OPENFILENAME_SIZE_VERSION_400W
    #else
        #define _OPENFILENAME_SIZE_VERSION_400  _OPENFILENAME_SIZE_VERSION_400A
    #endif // !UNICODE
#else
    #error _WIN32_WINNT seams not to be valid.
#endif // (_WIN32_WINNT >= 0x0500)


//-------------------------------------------------------------
// A simple wrapper class around the Win32 GetOpenFileName API.
// This class is not thread-safe and only one instance at a
// time is allowed
//-------------------------------------------------------------

class CFileOpenDialog
{
public:
    // ctor
    // bFileOpenDialog idicates if we want a FileOpen or FileSave
    // dialog
    // dwFlags see OPENFILENAME
    // dwTemplateId - an ID for custom templates
    // hInstance    - an instance handle for the module
    // which provides the custom template, unused if dwTemplateId
    // is 0
    CFileOpenDialog(
        bool bFileOpenDialog = sal_True,
        sal_uInt32 dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        sal_uInt32 dwTemplateId = 0,
        HINSTANCE hInstance = 0);

    virtual ~CFileOpenDialog();

    virtual void SAL_CALL setTitle(const rtl::OUString& aTitle);

    // to set a filter string using the M$ format
    // e.g. FltName\0*.txt;*.rtf\0...\0\0
    void SAL_CALL setFilter(const rtl::OUString& aFilter);

    // set the index of the current filter when the
    // dialog is about to shown, the index starts with 1
    // the function succeeded if the given filter index
    // is greater than zero and is a valid position
    // within filter string that was previously set
    bool SAL_CALL setFilterIndex(sal_uInt32 aIndex);

    // get the index of the currently selected filter
    // the index of the returned filter starts with 1
    sal_uInt32 SAL_CALL getSelectedFilterIndex() const;

    // set the name and optional the path of the
    // file that will be initially be shown when
    // the dialog will be displayed
    virtual void SAL_CALL setDefaultName(const rtl::OUString& aName);

    // set the initial directory
    virtual void SAL_CALL setDisplayDirectory(const rtl::OUString& aDirectory);

    // returns only the path of the selected file
    virtual rtl::OUString SAL_CALL getLastDisplayDirectory() const;

    // returns the full file name including drive letter, path
    // file name and file extension
    virtual rtl::OUString SAL_CALL getFullFileName() const;

    // returns the file name and the file extension without
    // drive letter and path
    rtl::OUString SAL_CALL getFileName() const;

    // returns the file extension of the selected file
    rtl::OUString SAL_CALL getFileExtension();

    // set a default extension, only the first three letters of
    // the given extension will be used; the given extension
    // should not contain a '.'
    void SAL_CALL setDefaultFileExtension(const rtl::OUString& aExtension);

    // enables or disables the multiselection mode for
    // the FileOpen/FileSave dialog
    void SAL_CALL setMultiSelectionMode(bool bMode);

    // returns whether multi-selection mode is enabled or not
    bool SAL_CALL getMultiSelectionMode() const;

    // shows the dialog, calls preModal before
    // showing the dialog and postModal after
    // showing the dialog
    // the method returns:
    //  0 - when the dialog was canceled by the user
    //  1 - when the dialog was closed with ok
    // -1 - when an error occurred
    sal_Int16 SAL_CALL doModal();

    // returns the last dialog error that occurred
    sal_uInt32 SAL_CALL getLastDialogError() const;

    // retrievs the currently selected file
    // including path and drive information
    // can be called only if the dialog is
    // already displayed
    rtl::OUString SAL_CALL getCurrentFilePath() const;

    // retrievs the currently selected folder
    rtl::OUString SAL_CALL getCurrentFolderPath() const;

    // retrievs the currently selected file name
    // without drive and path
    rtl::OUString SAL_CALL getCurrentFileName() const;

protected:
    // have to be overwritten when subclasses
    // want to do special pre- and post-modal
    // processing

    // if preModal return true processing will
    // continue else doModal exit without showing
    // a dialog and returns -1
    virtual bool SAL_CALL preModal();

    // post modal processing
    // the function should accept only values returned from
    // doModal and act appropriately
    virtual void SAL_CALL postModal(sal_Int16 nDialogResult);

    // message handler, to be overwritten by subclasses
    virtual sal_uInt32 SAL_CALL onShareViolation(const rtl::OUString& aPathName);
    virtual sal_uInt32 SAL_CALL onFileOk();
    virtual void SAL_CALL onSelChanged(HWND hwndListBox);
    virtual void SAL_CALL onHelp();

    // only called back if OFN_EXPLORER is set
    virtual void SAL_CALL onInitDone();
    virtual void SAL_CALL onFolderChanged();
    virtual void SAL_CALL onTypeChanged(sal_uInt32 nFilterIndex);

    virtual void SAL_CALL onInitDialog(HWND hwndDlg) = 0;

    virtual sal_uInt32 SAL_CALL onCtrlCommand(HWND hwndDlg, sal_uInt16 ctrlId, sal_uInt16 notifyCode);

    sal_uInt32 SAL_CALL onWMNotify(HWND hwndChild, LPOFNOTIFYW lpOfNotify);

    // we use non-virtual functions to do necessary work before
    // calling the virtual funtions (see Gamma: Template method)
    void SAL_CALL handleInitDialog(HWND hwndDlg, HWND hwndChild);

protected:

    // handle to the window of the
    // FileOpen/FileSave dialog
    // will be set on message
    // WM_INITDIALOG, before this
    // value is undefined
    HWND    m_hwndFileOpenDlg;
    HWND    m_hwndFileOpenDlgChild;

    _OPENFILENAME   m_ofn;

    // we connect the instance with the dialog window using
    // SetProp, with this function we can reconnect from
    // callback functions to this instance
    static CFileOpenDialog* SAL_CALL getCurrentInstance(HWND hwnd);

    void SAL_CALL centerPositionToParent() const;

private:
    // FileOpen or FileSaveDialog
    bool            m_bFileOpenDialog;
    rtl::OUString   m_dialogTitle;
    rtl::OUString   m_displayDirectory;
    rtl::OUString   m_defaultExtension;

    mutable rtl::OUStringBuffer m_filterBuffer;
    mutable rtl::OUStringBuffer m_fileTitleBuffer;
    mutable rtl::OUStringBuffer m_helperBuffer;
    mutable rtl::OUStringBuffer m_fileNameBuffer;

    CGetFileNameWrapper m_GetFileNameWrapper;

    WNDPROC             m_pfnBaseDlgProc;

    // callback function
    static UINT_PTR CALLBACK ofnHookProc(
        HWND hChildDlg, // handle to child dialog box
        UINT uiMsg,     // message identifier
        WPARAM wParam,  // message parameter
        LPARAM lParam   // message parameter
    );

    // we have to subclass the dialog in order
    // to clean up the window property we are
    // using to connect the window with a class
    // instance in WM_NCDESTROY
    static LRESULT CALLBACK BaseDlgProc(
        HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam );

private:
    // avoid copy and assignment
    CFileOpenDialog(const CFileOpenDialog&);
    CFileOpenDialog& operator=(const CFileOpenDialog&);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
