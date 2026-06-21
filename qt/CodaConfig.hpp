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
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */