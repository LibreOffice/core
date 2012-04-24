/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
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

    HWND hwndMSI = FindWindow(TEXT("MsiDialogCloseClass"), NULL);
    HWND hwndTV = FindWindowEx(hwndMSI, NULL, TEXT("SysTreeView32"), NULL);
    HTREEITEM treeRoot = TreeView_GetRoot(hwndTV);
    HTREEITEM optional = TreeView_GetNextSibling(hwndTV, treeRoot);
    HTREEITEM dicts = TreeView_GetChild(hwndTV, optional);
    TreeView_SortChildren(hwndTV, dicts, TRUE);
    HTREEITEM langs = TreeView_GetNextSibling(hwndTV, optional);
    TreeView_SortChildren(hwndTV, langs, TRUE);

	return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
