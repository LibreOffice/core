/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
    // Sort items (languages) in SelectionTree control, fdo#46355

    HWND hwndMSI = FindWindowW(L"MsiDialogCloseClass", NULL);
    if (hwndMSI == NULL)
    {
        OutputDebugStringA("SortTree: MsiDialogCloseClass not found\n");
        return ERROR_SUCCESS;
    }
    HWND hwndTV = FindWindowExW(hwndMSI, NULL, L"SysTreeView32", NULL);
    if (hwndTV == NULL)
    {
        OutputDebugStringA("SortTree: SysTreeView32 not found\n");
        return ERROR_SUCCESS;
    }
    HTREEITEM optional = TreeView_GetRoot(hwndTV);
    if (optional == NULL)
    {
        OutputDebugStringA("SortTree: Optional Components branch not found\n");
        return ERROR_SUCCESS;
    }
    HTREEITEM dicts = TreeView_GetChild(hwndTV, optional);
    if (dicts == NULL)
    {
        OutputDebugStringA("SortTree: Dictionaries branch not found\n");
        return ERROR_SUCCESS;
    }
    TreeView_SortChildren(hwndTV, dicts, TRUE);
    HTREEITEM langs = TreeView_GetNextSibling(hwndTV, optional);
    if (langs == NULL)
    {
        OutputDebugStringA("SortTree: Additional UI Languages branch not found\n");
        return ERROR_SUCCESS;
    }
    TreeView_SortChildren(hwndTV, langs, TRUE);

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
