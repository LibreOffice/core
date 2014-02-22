/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#include <commctrl.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

extern "C" UINT __stdcall SortTree(MSIHANDLE)
{
    

    HWND hwndMSI = FindWindow(TEXT("MsiDialogCloseClass"), NULL);
    if (hwndMSI == NULL)
    {
        OutputDebugString("SortTree: MsiDialogCloseClass not found\n");
        return ERROR_SUCCESS;
    }
    HWND hwndTV = FindWindowEx(hwndMSI, NULL, TEXT("SysTreeView32"), NULL);
    if (hwndTV == NULL)
    {
        OutputDebugString("SortTree: SysTreeView32 not found\n");
        return ERROR_SUCCESS;
    }
    HTREEITEM optional = TreeView_GetRoot(hwndTV);
    if (optional == NULL)
    {
        OutputDebugString("SortTree: Optional Components branch not found\n");
        return ERROR_SUCCESS;
    }
    HTREEITEM dicts = TreeView_GetChild(hwndTV, optional);
    if (dicts == NULL)
    {
        OutputDebugString("SortTree: Dictionaries branch not found\n");
        return ERROR_SUCCESS;
    }
    TreeView_SortChildren(hwndTV, dicts, TRUE);
    HTREEITEM langs = TreeView_GetNextSibling(hwndTV, optional);
    if (langs == NULL)
    {
        OutputDebugString("SortTree: Additional UI Languages branch not found\n");
        return ERROR_SUCCESS;
    }
    TreeView_SortChildren(hwndTV, langs, TRUE);

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
