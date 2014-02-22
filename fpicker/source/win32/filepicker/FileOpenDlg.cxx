/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <tchar.h>
#include <osl/diagnose.h>
#include "../misc/WinImplHelper.hxx"
#include "FileOpenDlg.hxx"





namespace /* private */
{
    
    
    
    
    
    
    
    
    
    
    const size_t MAX_FILENAME_BUFF_SIZE  = 32000;
    const size_t MAX_FILETITLE_BUFF_SIZE = 32000;
    const size_t MAX_FILTER_BUFF_SIZE    = 4096;

    const LPCTSTR CURRENT_INSTANCE = TEXT("CurrInst");

    
    
    

    inline bool is_current_process_window(HWND hwnd)
    {
        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);
        return (pid == GetCurrentProcessId());
    }

    HWND choose_parent_window()
    {
        HWND hwnd_parent = GetForegroundWindow();
        if (!is_current_process_window(hwnd_parent))
            hwnd_parent = GetDesktopWindow();

        return hwnd_parent;
    }
};


//


CFileOpenDialog::CFileOpenDialog(
    bool bFileOpenDialog,
    sal_uInt32 dwFlags,
    sal_uInt32 dwTemplateId,
    HINSTANCE hInstance) :
    m_hwndFileOpenDlg(0),
    m_hwndFileOpenDlgChild(0),
    m_bFileOpenDialog(bFileOpenDialog),
    m_filterBuffer(MAX_FILTER_BUFF_SIZE),
    m_fileTitleBuffer(MAX_FILETITLE_BUFF_SIZE),
    m_helperBuffer(MAX_FILENAME_BUFF_SIZE),
    m_fileNameBuffer(MAX_FILENAME_BUFF_SIZE),
    m_pfnBaseDlgProc(0)
{
    
    ZeroMemory(&m_ofn, sizeof(m_ofn));
    m_ofn.lStructSize = sizeof(m_ofn);

    
    
    m_ofn.Flags |= dwFlags |
                   OFN_EXPLORER |
                   OFN_ENABLEHOOK |
                   OFN_HIDEREADONLY |
                   OFN_PATHMUSTEXIST |
                   OFN_FILEMUSTEXIST |
                   OFN_OVERWRITEPROMPT |
                   OFN_ENABLESIZING |
                   OFN_DONTADDTORECENT; 

    
    
    m_ofn.hwndOwner = choose_parent_window();

    m_ofn.lpstrFile = reinterpret_cast<LPTSTR>(const_cast<sal_Unicode*>(m_fileNameBuffer.getStr()));
    m_ofn.nMaxFile  = m_fileNameBuffer.getCapacity();

    m_ofn.lpstrFileTitle = reinterpret_cast<LPTSTR>(const_cast<sal_Unicode*>(m_fileTitleBuffer.getStr()));
    m_ofn.nMaxFileTitle  = m_fileTitleBuffer.getCapacity();

    m_ofn.lpfnHook = CFileOpenDialog::ofnHookProc;

    

    if (dwTemplateId)
    {
        OSL_ASSERT(hInstance);

        m_ofn.Flags          |= OFN_ENABLETEMPLATE;
        m_ofn.lpTemplateName  = MAKEINTRESOURCE(dwTemplateId);
        m_ofn.hInstance       = hInstance;
    }

    
    m_ofn.lCustData = reinterpret_cast<sal_IntPtr>(this);
}


//


CFileOpenDialog::~CFileOpenDialog()
{
}


//


void SAL_CALL CFileOpenDialog::setTitle(const OUString& aTitle)
{
    m_dialogTitle = aTitle;
    m_ofn.lpstrTitle = reinterpret_cast<LPCTSTR>(m_dialogTitle.getStr());
}


//


void CFileOpenDialog::setFilter(const OUString& aFilter)
{
    
    
    
    m_filterBuffer.ensureCapacity(aFilter.getLength());
    m_filterBuffer.setLength(0);
    m_filterBuffer.append(aFilter);
    m_ofn.lpstrFilter = reinterpret_cast<LPCTSTR>(m_filterBuffer.getStr());
}


//


bool CFileOpenDialog::setFilterIndex(sal_uInt32 aIndex)
{
    OSL_ASSERT(aIndex > 0);
    m_ofn.nFilterIndex = aIndex;
    return sal_True;
}


//


sal_uInt32 CFileOpenDialog::getSelectedFilterIndex() const
{
    return m_ofn.nFilterIndex;
}


//


void SAL_CALL CFileOpenDialog::setDefaultName(const OUString& aName)
{
    m_fileNameBuffer.setLength(0);
    m_fileNameBuffer.append(aName);
    m_ofn.lpstrFile = reinterpret_cast<LPTSTR>(const_cast<sal_Unicode*>(m_fileNameBuffer.getStr()));
}


//


void SAL_CALL CFileOpenDialog::setDisplayDirectory(const OUString& aDirectory)
{
    m_displayDirectory = aDirectory;
    m_ofn.lpstrInitialDir = reinterpret_cast<LPCTSTR>(m_displayDirectory.getStr());
}


//


OUString SAL_CALL CFileOpenDialog::getLastDisplayDirectory() const
{
    return m_displayDirectory;
}


//


OUString SAL_CALL CFileOpenDialog::getFullFileName() const
{
    return OUString(m_fileNameBuffer.getStr(),
        _wcslenex(m_fileNameBuffer.getStr()));
}


//


OUString SAL_CALL CFileOpenDialog::getFileName() const
{
    return OUString(m_fileTitleBuffer.getStr());
}


//


OUString CFileOpenDialog::getFileExtension()
{
    if (m_ofn.nFileExtension)
        return OUString(m_fileNameBuffer.getStr() + m_ofn.nFileExtension,
            rtl_ustr_getLength(m_fileNameBuffer.getStr() + m_ofn.nFileExtension));

    return OUString();
}


//


void CFileOpenDialog::setDefaultFileExtension(const OUString& aExtension)
{
    m_defaultExtension = aExtension;
    m_ofn.lpstrDefExt  = reinterpret_cast<LPCTSTR>(m_defaultExtension.getStr());
}


//


void SAL_CALL CFileOpenDialog::setMultiSelectionMode(bool bMode)
{
    if (bMode)
        m_ofn.Flags |= OFN_ALLOWMULTISELECT;
    else
        m_ofn.Flags &= ~OFN_ALLOWMULTISELECT;
}


//


bool SAL_CALL CFileOpenDialog::getMultiSelectionMode() const
{
    return ((m_ofn.Flags & OFN_ALLOWMULTISELECT) > 0);
}


//


sal_Int16 SAL_CALL CFileOpenDialog::doModal()
{
    sal_Int16 nRC = -1;

    
    if (preModal())
    {
        bool bRet;

        if (m_bFileOpenDialog)
            bRet = m_GetFileNameWrapper.getOpenFileName(
                reinterpret_cast<LPOPENFILENAME>(&m_ofn));
        else
            bRet = m_GetFileNameWrapper.getSaveFileName(
                reinterpret_cast<LPOPENFILENAME>(&m_ofn));

        nRC = 1;

        if (!bRet)
            nRC = (0 == m_GetFileNameWrapper.commDlgExtendedError()) ? 0 : -1;

        
        postModal(nRC);
    }

    return nRC;
}


//


sal_uInt32 SAL_CALL CFileOpenDialog::getLastDialogError() const
{
    return CommDlgExtendedError();
}


//


bool SAL_CALL CFileOpenDialog::preModal()
{
    return sal_True;
}


//


void SAL_CALL CFileOpenDialog::postModal(sal_Int16 nDialogResult)
{
    OSL_ASSERT((-1 <= nDialogResult) && (nDialogResult <= 1));

    if (1 == nDialogResult)
    {
        
        
        
        m_displayDirectory = OUString(reinterpret_cast<const sal_Unicode*>(m_ofn.lpstrFile),m_ofn.nFileOffset);
    }
}


//


OUString SAL_CALL CFileOpenDialog::getCurrentFilePath() const
{
    OSL_ASSERT(IsWindow(m_hwndFileOpenDlg));

    LPARAM nLen = SendMessage(
        m_hwndFileOpenDlg,
        CDM_GETFILEPATH,
        m_helperBuffer.getCapacity(),
        reinterpret_cast<LPARAM>(m_helperBuffer.getStr()));

    if (nLen > 0)
    {
        m_helperBuffer.setLength((nLen * sizeof(sal_Unicode)) - 1);
        return OUString(m_helperBuffer.getStr());
    }
    return OUString();
}


//


OUString SAL_CALL CFileOpenDialog::getCurrentFolderPath() const
{
    OSL_ASSERT(IsWindow(m_hwndFileOpenDlg));

    LPARAM nLen = SendMessage(
        m_hwndFileOpenDlg,
        CDM_GETFOLDERPATH,
        m_helperBuffer.getCapacity(),
        reinterpret_cast<LPARAM>(m_helperBuffer.getStr()));

    if (nLen > 0)
    {
        m_helperBuffer.setLength((nLen * sizeof(sal_Unicode)) - 1);
        return OUString(m_helperBuffer.getStr());
    }
    return OUString();
}


//


OUString SAL_CALL CFileOpenDialog::getCurrentFileName() const
{
    OSL_ASSERT(IsWindow(m_hwndFileOpenDlg));

    LPARAM nLen = SendMessage(
        m_hwndFileOpenDlg,
        CDM_GETSPEC,
        m_helperBuffer.getCapacity(),
        reinterpret_cast<LPARAM>(m_helperBuffer.getStr()));

    if (nLen > 0)
    {
        m_helperBuffer.setLength((nLen * sizeof(sal_Unicode)) - 1);
        return OUString(m_helperBuffer.getStr());
    }
    return OUString();
}


//


sal_uInt32 SAL_CALL CFileOpenDialog::onShareViolation(const OUString&)
{
    return 0;
}


//


sal_uInt32 SAL_CALL CFileOpenDialog::onFileOk()
{
    return 0;
}


//


void SAL_CALL CFileOpenDialog::onSelChanged(HWND)
{
}


//


void SAL_CALL CFileOpenDialog::onHelp()
{
}


//


void SAL_CALL CFileOpenDialog::onInitDone()
{
    centerPositionToParent();
}


//


void SAL_CALL CFileOpenDialog::onFolderChanged()
{
}


//


void SAL_CALL CFileOpenDialog::onTypeChanged(sal_uInt32)
{
}


//


sal_uInt32 SAL_CALL CFileOpenDialog::onCtrlCommand(HWND, sal_uInt16, sal_uInt16)
{
    return 0;
}


//


sal_uInt32 SAL_CALL CFileOpenDialog::onWMNotify( HWND, LPOFNOTIFY lpOfNotify )
{
    switch(lpOfNotify->hdr.code)
    {
    case CDN_SHAREVIOLATION:
        return onShareViolation(reinterpret_cast<const sal_Unicode*>(lpOfNotify->pszFile));

    case CDN_FILEOK:
        return onFileOk();

    case CDN_SELCHANGE:
        onSelChanged(lpOfNotify->hdr.hwndFrom);
        break;

    case CDN_HELP:
        onHelp();
        break;

    case CDN_INITDONE:
        onInitDone();
        break;

    case CDN_FOLDERCHANGE:
        onFolderChanged();
        break;

    case CDN_TYPECHANGE:
        m_ofn.nFilterIndex = lpOfNotify->lpOFN->nFilterIndex;
        onTypeChanged(lpOfNotify->lpOFN->nFilterIndex);
        break;
    }

    return 0;
}


//


void SAL_CALL CFileOpenDialog::handleInitDialog(HWND hwndDlg, HWND hwndChild)
{
    m_hwndFileOpenDlg      = hwndDlg;
    m_hwndFileOpenDlgChild = hwndChild;

    OSL_ASSERT(GetParent(hwndChild) == hwndDlg);

    
    
    onInitDialog(hwndDlg);
}


//


UINT_PTR CALLBACK CFileOpenDialog::ofnHookProc(
    HWND hChildDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndDlg = GetParent(hChildDlg);
    CFileOpenDialog* pImpl = NULL;

    switch( uiMsg )
    {
    case WM_INITDIALOG:
        {
            _LPOPENFILENAME lpofn = reinterpret_cast<_LPOPENFILENAME>(lParam);
            pImpl = reinterpret_cast<CFileOpenDialog*>(lpofn->lCustData);
            OSL_ASSERT(pImpl);

            
            pImpl->m_pfnBaseDlgProc =
                reinterpret_cast<WNDPROC>(
                    SetWindowLongPtr(
                        hwndDlg,
                        GWLP_WNDPROC,
                        reinterpret_cast<LONG_PTR>(CFileOpenDialog::BaseDlgProc)));
            
            SetProp(hwndDlg, CURRENT_INSTANCE, pImpl);
            pImpl->handleInitDialog(hwndDlg, hChildDlg);
        }
        return 0;

    case WM_NOTIFY:
        {
            pImpl = getCurrentInstance(hwndDlg);
            return pImpl->onWMNotify(
                hChildDlg, reinterpret_cast<LPOFNOTIFY>(lParam));
        }

    case WM_COMMAND:
        {
            pImpl = getCurrentInstance(hwndDlg);
            OSL_ASSERT(pImpl);

            return pImpl->onCtrlCommand(
                hChildDlg, LOWORD(wParam), HIWORD(lParam));
        }
    }

    return 0;
}


//


LRESULT CALLBACK CFileOpenDialog::BaseDlgProc(
    HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam)
{
    CFileOpenDialog* pImpl = 0;

    if (WM_NCDESTROY == wMessage)
    {
        pImpl = reinterpret_cast<CFileOpenDialog*>(
            RemoveProp(hWnd,CURRENT_INSTANCE));

        SetWindowLongPtr(hWnd, GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(pImpl->m_pfnBaseDlgProc));
    }
    else
    {
        pImpl = getCurrentInstance(hWnd);
    }

    OSL_ASSERT(pImpl);

    return CallWindowProc(
        reinterpret_cast<WNDPROC>(pImpl->m_pfnBaseDlgProc),
        hWnd,wMessage,wParam,lParam);
}


//


CFileOpenDialog* SAL_CALL CFileOpenDialog::getCurrentInstance(HWND hwnd)
{
    OSL_ASSERT(IsWindow( hwnd));
    return reinterpret_cast<CFileOpenDialog*>(
        GetProp(hwnd, CURRENT_INSTANCE));
}


//


void SAL_CALL CFileOpenDialog::centerPositionToParent() const
{
    OSL_PRECOND(IsWindow(m_hwndFileOpenDlg), "no dialog window, call method only after or in onInitDone");

    HWND hwndParent = m_ofn.hwndOwner;

    if (!IsWindow(hwndParent))
        hwndParent = GetDesktopWindow();

    OSL_ASSERT(IsWindow(hwndParent));

    RECT rcPar;
    GetWindowRect(hwndParent, &rcPar);

    RECT rcDlg;
    GetWindowRect(m_hwndFileOpenDlg, &rcDlg);

    int lDlgW = rcDlg.right  - rcDlg.left;
    int lDlgH = rcDlg.bottom - rcDlg.top;

    int x = (rcPar.left + rcPar.right  - lDlgW) / 2;
    int y = (rcPar.top  + rcPar.bottom - lDlgH) / 2;

    HDC hdc = GetDC(m_hwndFileOpenDlg);

    int hResol = GetDeviceCaps(hdc, HORZRES);
    int vResol = GetDeviceCaps(hdc, VERTRES);

    ReleaseDC(m_hwndFileOpenDlg, hdc);

    if (x < 0)
        x = 0;
    else if ((x + lDlgW) > hResol)
        x = hResol - lDlgW;

    if (y < 0)
        y = 0;
    else if ((y + lDlgH) > vResol)
        y = vResol - lDlgH;

    SetWindowPos(
        m_hwndFileOpenDlg,
        NULL, x, y, 0, 0,
        SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
