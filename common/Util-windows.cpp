/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <random>

#include <process.h>
#include <Windows.h>

#include <common/Util.hpp>

#include <cstdio>

namespace Util
{
    namespace rng
    {
        std::vector<char> getBytes(const std::size_t length)
        {
            std::vector<char> v(length);

            static std::random_device rd;
            size_t offset;
            size_t byteoffset = 0;
            std::random_device::result_type buffer;
            for (offset = 0; offset < length; offset++)
            {
                if (offset % sizeof(std::random_device::result_type) == 0)
                    buffer = rd();
                v[offset] = (buffer >> byteoffset) & 0xFF;
                byteoffset = (byteoffset + 8) % (8 * sizeof(std::random_device::result_type));
            }

            return v;
        }
    } // namespace rng

    std::tm *time_t_to_gmtime(std::time_t t, std::tm& tm)
    {
        if (gmtime_s(&tm, &t) != 0)
            return nullptr;
        return &tm;
    }

    std::wstring string_to_wide_string(const std::string_view string)
    {
        if (string.empty())
        {
            return L"";
        }

        const auto size_needed = MultiByteToWideChar(CP_UTF8, 0, string.data(), (int)string.size(), nullptr, 0);
        if (size_needed <= 0)
        {
            throw std::runtime_error("MultiByteToWideChar() failed: " + std::to_string(size_needed));
        }

        std::wstring result(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, string.data(), (int)string.size(), result.data(), size_needed);

        return result;
    }

    std::string wide_string_to_string(const std::wstring& wide_string)
    {
        if (wide_string.empty())
        {
            return "";
        }

        const auto size_needed = WideCharToMultiByte(CP_UTF8, 0, wide_string.data(), (int)wide_string.size(), nullptr, 0, nullptr, nullptr);
        if (size_needed <= 0)
        {
            throw std::runtime_error("WideCharToMultiByte() failed: " + std::to_string(size_needed));
        }

        std::string result(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wide_string.data(), (int)wide_string.size(), result.data(), size_needed, nullptr, nullptr);

        return result;
    }

#define CODA_ERROR_NAME(code)                                                                      \
    case code:                                                                                     \
        return #code

    std::string win32ErrorName(unsigned long error)
    {
        // There is no call that gives the symbolic name of an error code, only FormatMessage(),
        // which gives a localized text. These are the codes that the failures we log can have:
        // file access, process creation, windows, the clipboard, base64 conversion, and the
        // general ones that any call can set.
        switch (error)
        {
            CODA_ERROR_NAME(ERROR_SUCCESS);
            CODA_ERROR_NAME(ERROR_INVALID_FUNCTION);
            CODA_ERROR_NAME(ERROR_FILE_NOT_FOUND);
            CODA_ERROR_NAME(ERROR_PATH_NOT_FOUND);
            CODA_ERROR_NAME(ERROR_TOO_MANY_OPEN_FILES);
            CODA_ERROR_NAME(ERROR_ACCESS_DENIED);
            CODA_ERROR_NAME(ERROR_INVALID_HANDLE);
            CODA_ERROR_NAME(ERROR_NOT_ENOUGH_MEMORY);
            CODA_ERROR_NAME(ERROR_INVALID_DATA);
            CODA_ERROR_NAME(ERROR_OUTOFMEMORY);
            CODA_ERROR_NAME(ERROR_INVALID_DRIVE);
            CODA_ERROR_NAME(ERROR_WRITE_PROTECT);
            CODA_ERROR_NAME(ERROR_NOT_READY);
            CODA_ERROR_NAME(ERROR_SHARING_VIOLATION);
            CODA_ERROR_NAME(ERROR_LOCK_VIOLATION);
            CODA_ERROR_NAME(ERROR_NOT_SUPPORTED);
            CODA_ERROR_NAME(ERROR_BAD_NETPATH);
            CODA_ERROR_NAME(ERROR_NETWORK_ACCESS_DENIED);
            CODA_ERROR_NAME(ERROR_BAD_NET_NAME);
            CODA_ERROR_NAME(ERROR_FILE_EXISTS);
            CODA_ERROR_NAME(ERROR_INVALID_PARAMETER);
            CODA_ERROR_NAME(ERROR_DISK_FULL);
            CODA_ERROR_NAME(ERROR_INSUFFICIENT_BUFFER);
            CODA_ERROR_NAME(ERROR_INVALID_NAME);
            CODA_ERROR_NAME(ERROR_ALREADY_EXISTS);
            CODA_ERROR_NAME(ERROR_FILENAME_EXCED_RANGE);
            CODA_ERROR_NAME(ERROR_BAD_EXE_FORMAT);
            CODA_ERROR_NAME(ERROR_MORE_DATA);
            CODA_ERROR_NAME(ERROR_DIRECTORY);
            CODA_ERROR_NAME(ERROR_ELEVATION_REQUIRED);
            CODA_ERROR_NAME(ERROR_INVALID_WINDOW_HANDLE);
            CODA_ERROR_NAME(ERROR_INVALID_MENU_HANDLE);
            CODA_ERROR_NAME(ERROR_CLASS_DOES_NOT_EXIST);
            CODA_ERROR_NAME(ERROR_CANNOT_FIND_WND_CLASS);
            CODA_ERROR_NAME(ERROR_TLW_WITH_WSCHILD);
            CODA_ERROR_NAME(ERROR_CLIPBOARD_NOT_OPEN);
            CODA_ERROR_NAME(ERROR_NO_SYSTEM_RESOURCES);
            CODA_ERROR_NAME(ERROR_NOT_ENOUGH_QUOTA);
            CODA_ERROR_NAME(ERROR_TIMEOUT);
            CODA_ERROR_NAME(ERROR_LOGON_FAILURE);
            CODA_ERROR_NAME(ERROR_INVALID_STATE);
            CODA_ERROR_NAME(ERROR_INVALID_WINDOW_STYLE);
            default:
                return std::to_string(error);
        }
    }

    std::string hresultName(long result)
    {
        switch (result)
        {
            CODA_ERROR_NAME(S_OK);
            CODA_ERROR_NAME(S_FALSE);
            CODA_ERROR_NAME(E_NOTIMPL);
            CODA_ERROR_NAME(E_NOINTERFACE);
            CODA_ERROR_NAME(E_POINTER);
            CODA_ERROR_NAME(E_ABORT);
            CODA_ERROR_NAME(E_FAIL);
            CODA_ERROR_NAME(E_UNEXPECTED);
            CODA_ERROR_NAME(E_ACCESSDENIED);
            CODA_ERROR_NAME(E_HANDLE);
            CODA_ERROR_NAME(E_OUTOFMEMORY);
            CODA_ERROR_NAME(E_INVALIDARG);
            CODA_ERROR_NAME(E_PENDING);
            CODA_ERROR_NAME(CO_E_NOTINITIALIZED);
            CODA_ERROR_NAME(RPC_E_DISCONNECTED);
            CODA_ERROR_NAME(RPC_E_WRONG_THREAD);
            CODA_ERROR_NAME(DRAGDROP_E_INVALIDHWND);
            CODA_ERROR_NAME(DRAGDROP_E_ALREADYREGISTERED);
            default:
                break;
        }

        if (HRESULT_FACILITY(result) == FACILITY_WIN32)
            return "HRESULT_FROM_WIN32(" + win32ErrorName(HRESULT_CODE(result)) + ')';

        char hex[16];
        std::snprintf(hex, sizeof(hex), "0x%08lX", static_cast<unsigned long>(result));
        return hex;
    }

#undef CODA_ERROR_NAME
} // namespace Util

namespace ProcUtil
{
    long getProcessId()
    {
        return _getpid();
    }
} // namespace ProcUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
