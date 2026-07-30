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
#include <common/Util.hpp>

#include <o3tl/safeint.hxx>

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

                assert(o3tl::make_unsigned(b) <= nbytes);

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
} // namespace Util

namespace ProcUtil
{
    long getProcessId()
    {
        return getpid();
    }
} // namespace ProcUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
