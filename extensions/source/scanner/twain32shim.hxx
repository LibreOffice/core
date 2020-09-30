/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <prewin.h>
#include <Shlobj.h>
#include <postwin.h>
#include <exception>
#include <string>
#include <sstream>
#include <iomanip>

// Don't use WM_USER

// Notifications from shim to parent; wParam = event id
#define WM_TWAIN_EVENT (WM_USER + 1)

// lParam is HWND
#define TWAIN_EVENT_NOTIFYHWND 0

// lParam is result (bool indicating success)
#define TWAIN_EVENT_REQUESTRESULT 1

// lParam is ignored
#define TWAIN_EVENT_NONE 10
#define TWAIN_EVENT_QUIT 11
#define TWAIN_EVENT_SCANNING 12

// lParam is HANDLE to shared file mapping valid in context of parent process
#define TWAIN_EVENT_XFER 13

// Requests from parent to shim; wParam = request id
#define WM_TWAIN_REQUEST (WM_USER + 2)

#define TWAIN_REQUEST_QUIT 0 // Destroy()
#define TWAIN_REQUEST_SELECTSOURCE 1
#define TWAIN_REQUEST_INITXFER 2

// messages starting from this are not to be used for interprocess communications
#define WM_SHIM_INTERNAL (WM_USER + 200)

template <typename IntType> std::string Num2Hex(IntType n)
{
    std::stringstream sMsg;
    sMsg << "0x" << std::uppercase << std::setfill('0') << std::setw(sizeof(n) * 2) << std::hex
         << n;
    return sMsg.str();
}

void ThrowWin32Error(const char* sFunc, DWORD nWin32Error)
{
    std::stringstream sMsg;
    sMsg << sFunc << " failed with Win32 error code " << Num2Hex(nWin32Error) << "!";

    throw std::exception(sMsg.str().c_str());
}

void ThrowLastError(const char* sFunc) { ThrowWin32Error(sFunc, GetLastError()); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
