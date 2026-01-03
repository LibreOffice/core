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

#include <common/Util.hpp>

#include <chrono>
#include <mutex>
#include <string>

/// A rotating access token with current and previous values.
/// Supports automatic time-based rotation and graceful validation of both tokens.
class RotatingToken
{
    mutable std::mutex _mutex;

    /// Token values: [0] is current, [1] is previous
    std::string _tokens[2];

    /// Token length in bytes (hex string will be 2x this)
    const size_t _tokenLengthBytes;

    /// How long before tokens should be rotated
    const std::chrono::milliseconds _rotationInterval;

    /// When the last rotation occurred
    std::chrono::steady_clock::time_point _lastRotation;

public:
    /// Create a rotating token with specified length and rotation interval.
    /// @param tokenLengthBytes Length of token in bytes (hex string will be 2x this)
    /// @param rotationInterval How often tokens should be rotated (0 = manual only)
    RotatingToken(size_t tokenLengthBytes = 16,
                  std::chrono::milliseconds rotationInterval = std::chrono::milliseconds::zero())
        : _tokenLengthBytes(tokenLengthBytes)
        , _rotationInterval(rotationInterval)
        , _lastRotation(std::chrono::steady_clock::now())
    {
        // Initialize with two random tokens
        rotate();
        rotate();
    }

    /// Get the current token value.
    std::string getCurrent() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _tokens[0];
    }

    /// Get the previous token value.
    std::string getPrevious() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _tokens[1];
    }

    /// Rotate the token: current becomes previous, new current is generated.
    void rotate()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _tokens[1] = _tokens[0];
        _tokens[0] = Util::rng::getHexString(_tokenLengthBytes);
        _lastRotation = std::chrono::steady_clock::now();
    }

    /// Check if rotation is needed based on the configured interval.
    /// @param now Current time (for testing or efficiency when called repeatedly)
    /// @return true if rotation interval has elapsed
    bool needsRotation(std::chrono::steady_clock::time_point now =
                           std::chrono::steady_clock::now()) const
    {
        if (_rotationInterval.count() <= 0)
            return false;

        std::lock_guard<std::mutex> lock(_mutex);
        return (now - _lastRotation) >= _rotationInterval;
    }

    /// Rotate if the rotation interval has elapsed.
    /// @return true if rotation was performed
    bool rotateIfNeeded()
    {
        if (_rotationInterval.count() <= 0)
            return false;

        const auto now = std::chrono::steady_clock::now();

        std::lock_guard<std::mutex> lock(_mutex);
        if ((now - _lastRotation) >= _rotationInterval)
        {
            _tokens[1] = _tokens[0];
            _tokens[0] = Util::rng::getHexString(_tokenLengthBytes);
            _lastRotation = now;
            return true;
        }
        return false;
    }

    /// Check if a tag matches either the current or previous token.
    /// @param tag The tag to validate
    /// @return true if the tag matches current or previous token
    bool matches(const std::string& tag) const
    {
        if (tag.empty())
            return false;

        std::lock_guard<std::mutex> lock(_mutex);
        return _tokens[0] == tag || _tokens[1] == tag;
    }

    /// Get the time since the last rotation.
    std::chrono::milliseconds timeSinceRotation() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - _lastRotation);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
