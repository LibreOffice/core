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

    // Start ready to edit, for the --edit flag.
    bool _startInEditMode = false;

public:
    static CodaConfig& instance()
    {
        static CodaConfig config;
        return config;
    }

    bool isForcedReadOnly() const { return _forcedReadOnly; }
    void setForcedReadOnly(bool value) { _forcedReadOnly = value; }

    // Whether a document that is opened from a file starts ready to edit. It starts read-only
    // otherwise, with the Edit button waiting to be pressed.
    bool startsInEditMode() const { return _startInEditMode; }
    void setStartInEditMode(bool value) { _startInEditMode = value; }

    // Whether a Draw or Impress document is drawn from vector primitives instead of tiles.
    // CODA_VECTOR asks for that when it holds anything.
    bool wantsVector() const
    {
        static const bool wanted = []
        {
            const char* env = std::getenv("CODA_VECTOR");
            return env && *env;
        }();
        return wanted;
    }

    // The round trip in milliseconds that the messages between the page and the engine are held
    // back by, as on a slow connection, from the start. CODA_DELAY asks for that when it holds a
    // whole number above zero. Zero means the messages are not held back.
    int messageDelay() const
    {
        static const int delay = []
        {
            const char* env = std::getenv("CODA_DELAY");
            if (!env || !*env)
                return 0;
            char* end = nullptr;
            const long value = std::strtol(env, &end, 10);
            if (*end != '\0' || value < 0 || value > INT_MAX)
                return 0;
            return static_cast<int>(value);
        }();
        return delay;
    }

    // How many document views may hold a web engine renderer at once. A window keeps
    // the most recently used views and drops the renderers of the others, which stays
    // within one process's memory as more documents are opened. Zero means no limit.
    // CODA_LIVE_VIEWS overrides the default when it holds a whole number.
    int liveViewLimit() const
    {
        static const int limit = []
        {
            constexpr int defaultLimit = 5;
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
