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

#pragma once

#include <climits>
#include <cstdlib>

class CodaConfig
{
    CodaConfig() = default;

    // Disable editing mode when opening CODA with --readonly flag.
    bool _forcedReadOnly = false;

public:
    static CodaConfig& instance()
    {
        static CodaConfig config;
        return config;
    }

    bool isForcedReadOnly() const { return _forcedReadOnly; }
    void setForcedReadOnly(bool value) { _forcedReadOnly = value; }

    // How many document views may hold a web engine renderer at once. A window keeps
    // the most recently used views and drops the renderers of the others, which stays
    // within one process's memory as more documents are opened. Zero means no limit and
    // is the default. CODA_LIVE_VIEWS sets a limit when it holds a whole number.
    int liveViewLimit() const
    {
        static const int limit = []
        {
            constexpr int defaultLimit = 0;
            const char* env = std::getenv("CODA_LIVE_VIEWS");
            if (!env || !*env)
                return defaultLimit;
            char* end = nullptr;
            const long value = std::strtol(env, &end, 10);
            if (*end != '\0' || value < 0 || value > INT_MAX)
                return defaultLimit;
            return static_cast<int>(value);
        }();
        return limit;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
