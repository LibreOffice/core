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
 * Test logging utilities and timing functions.
 */

#pragma once

#include <common/Log.hpp>

namespace helpers
{
inline std::chrono::steady_clock::time_point& getTestStartTime()
{
    static auto TestStartTime = std::chrono::steady_clock::now();

    return TestStartTime;
}

inline void resetTestStartTime() { getTestStartTime() = std::chrono::steady_clock::now(); }

inline std::chrono::milliseconds timeSinceTestStartMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()
                                                                 - getTestStartTime());
}

} // namespace helpers

#if ENABLE_DEBUG
#define TST_LOG_NAME(NAME, X)                                                                      \
    LOG_TST(NAME << (NAME != std::string_view(__func__)                                            \
                         ? (" [" + std::string(__func__) + "]")                                    \
                         : "")                                                                     \
                 << " (+" << helpers::timeSinceTestStartMs() << "): " << std::boolalpha << X)
#else // Disable test logs in release.
#define TST_LOG_NAME(NAME, X)                                                                      \
    do                                                                                             \
    {                                                                                              \
        /* silence '-Werror=unused-variable' */                                                    \
        (void)NAME;                                                                                \
        std::stringstream dummyStringstream;                                                       \
        dummyStringstream << X;                                                                    \
    } while (0)
#endif // !ENABLE_DEBUG

/// Used to log the name of the test and the time since starting to run the tests.
#define TST_LOG(X) TST_LOG_NAME(testname, X)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
