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

#include <common/Util.hpp>

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
} // namespace Util

namespace ProcUtil
{
    long getProcessId()
    {
        return _getpid();
    }
} // namespace ProcUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
