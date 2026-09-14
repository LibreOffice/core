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

/*
 * Unix-specific utility implementations.
 * Functions: getURandom(), getBytes() - Random number generation
 */

#include <config.h>

#include <common/Log.hpp>
#include <common/NumUtil.hpp>
#include <common/ProcUtil.hpp>
#include <common/Util.hpp>

#include <cctype>
#include <chrono>
#include <cstdlib>
#include <string>

#include <fcntl.h>
#include <time.h>
#include <unistd.h>

namespace Util
{
    namespace rng
    {
        int getURandom()
        {
            static int urandom = open("/dev/urandom", O_RDONLY);
            if (urandom < 0)
            {
                LOG_SYS("Failed to source hard random numbers");
                fprintf(stderr, "No adequate source of randomness");
                abort();
                // Potentially dangerous to continue without randomness
            }
            return urandom;
        }

        // Since we have a fd always open to /dev/urandom
        // 'read' is hopefully no less efficient than getrandom.
        std::vector<char> getBytes(const std::size_t length)
        {
            std::vector<char> v(length);
            char* p = v.data();
            size_t nbytes = length;

            while (nbytes)
            {
                ssize_t b = read(getURandom(), p, nbytes);
                if (b <= 0)
                {
                    if (errno == EINTR)
                        continue;
                    break;
                }

                assert(NumUtil::makeUnsigned(b) <= nbytes);

                nbytes -= b;
                p += b;
            }

            size_t offset = p - v.data();
            if (offset < length)
            {
                fprintf(stderr, "No adequate source of randomness, "
                        "failed to read %zu bytes: with error %s\n",
                        length, strerror(errno));
                // Potentially dangerous to continue without randomness
                abort();
            }

            return v;
        }
    } // namespace rng

    std::tm *time_t_to_gmtime(std::time_t t, std::tm& tm)
    {
        return gmtime_r(&t, &tm);
    }

    // strptime and timegm are not in the MSVC runtime.
    bool isIso8601(const std::string& iso8601Time)
    {
        if (iso8601Time.empty())
            return false;

        std::tm tm;
        const char* trailing = strptime(iso8601Time.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
        if (!trailing)
            return false;

        // Accept what iso8601ToTimestamp can read: the bare second-precision
        // form, and the fractional-second form which requires at least one
        // digit after the dot.
        if (trailing[0] == '\0')
            return true;

        return trailing[0] == '.' && std::isdigit(static_cast<unsigned char>(trailing[1]));
    }

    std::chrono::system_clock::time_point iso8601ToTimestamp(const std::string& iso8601Time,
                                                             const std::string& logName)
    {
        std::chrono::system_clock::time_point timestamp;
        std::tm tm;
        const char* cstr = iso8601Time.c_str();
        const char* trailing;
        if (!(trailing = strptime(cstr, "%Y-%m-%dT%H:%M:%S", &tm)))
        {
            LOG_WRN(logName << " [" << iso8601Time << "] is in invalid format."
                               "Returning " << timestamp.time_since_epoch().count());
            return timestamp;
        }

        timestamp += std::chrono::seconds(timegm(&tm));
        if (trailing[0] == '\0')
            return timestamp;

        if (trailing[0] != '.')
        {
            LOG_WRN(logName << " [" << iso8601Time << "] is in invalid format."
                               ". Returning " << timestamp.time_since_epoch().count());
            return timestamp;
        }

        char* end = nullptr;
        const std::size_t us = strtoul(trailing + 1, &end, 10); // Skip the '.' and read as integer.

        std::size_t denominator = 1;
        for (const char* i = trailing + 1; i != end; i++)
        {
            denominator *= 10;
        }

        const std::size_t seconds_us = us * std::chrono::system_clock::period::den
                                       / std::chrono::system_clock::period::num / denominator;

        timestamp += std::chrono::system_clock::duration(seconds_us);

        return timestamp;
    }
} // namespace Util

namespace ProcUtil
{
    long getProcessId()
    {
        return getpid();
    }
} // namespace ProcUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
