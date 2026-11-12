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

#include <cstdlib>
#include <optional>
#include <string>
#include <utility>

/// An environment variable as it was when this was constructed. The destructor sets it back to
/// that value, or unsets it if it was not set, even when a test assertion throws.
class SavedEnvironmentVariable
{
    std::string _name;
    std::optional<std::string> _value;

public:
    explicit SavedEnvironmentVariable(std::string name)
        : _name(std::move(name))
    {
        if (const char* value = std::getenv(_name.c_str()))
            _value = value;
    }

    SavedEnvironmentVariable(const SavedEnvironmentVariable&) = delete;
    SavedEnvironmentVariable& operator=(const SavedEnvironmentVariable&) = delete;

    ~SavedEnvironmentVariable()
    {
        if (_value)
            ::setenv(_name.c_str(), _value->c_str(), 1);
        else
            ::unsetenv(_name.c_str());
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
