/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INCLUDED_COMPHELPER_WINSTART_HXX
#define INCLUDED_COMPHELPER_WINSTART_HXX

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * Launch a child process with the specified arguments.
 * @note argv[0] is ignored
 */
BOOL
WinLaunchChild(const wchar_t *exePath, int argc,
               wchar_t **argv, HANDLE userToken = nullptr,
               HANDLE *hProcess = nullptr);

wchar_t* MakeCommandLine(int argc, WCHAR **argv);

#endif
