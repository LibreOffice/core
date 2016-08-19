/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifdef XP_WIN
#include <windows.h>
#else
#include <limits.h>
#endif


BOOL
WinLaunchChild(const wchar_t *exePath, int argc,
               wchar_t **argv, HANDLE userToken = nullptr,
               HANDLE *hProcess = nullptr);

BOOL
WinLaunchChild(const wchar_t *exePath, int argc,
               char **argv, HANDLE userToken = nullptr,
               HANDLE *hProcess = nullptr);

wchar_t* MakeCommandLine(int argc, WCHAR **argv);
