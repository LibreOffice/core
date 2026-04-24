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

#include <mutex>
#include <string>
#include <unordered_map>

/// Process-wide, persistent string-keyed preferences store for CODA.
/// Backed by a flat JSON object on disk; write-through on every set.
/// Thread-safe.  The public API is intentionally Poco-free so that
/// CODA-W/CODA-M can reuse the class as-is.
class Prefs
{
public:
    /// Load prefs from @path.  If the file does not exist or cannot
    /// be parsed, start empty; subsequent set() calls will create it.
    explicit Prefs(std::string path);

    /// Return the value for @key, or empty string if not set.
    std::string get(const std::string& key) const;

    /// Set @key to @value and write through to the backing file.
    void set(const std::string& key, std::string value);

    /// Return the full prefs as a JSON string (flat object mapping
    /// string keys to string values).  Used by the bridge to hand
    /// the current state to JS in one shot at WebChannel connect.
    std::string serialize() const;

private:
    mutable std::mutex _mutex;
    std::unordered_map<std::string, std::string> _values;
    std::string _path;

    /// Serialise _values to JSON and write it to _path.  Caller must
    /// hold _mutex.
    void save() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
