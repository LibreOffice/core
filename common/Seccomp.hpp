/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <common/Protocol.hpp>

#include <cstdint>
#include <string_view>

namespace Seccomp
{
enum class Type : std::uint8_t
{
    KIT,
    WSD
};

/// Lock-down a process hard - @returns true on success.
bool lockdown(Type type);
}; // namespace Seccomp

namespace Rlimit
{
/// Lowest RLIMIT_NOFILE under which the kit can reasonably start.
/// This must accommodate the hundreds of fds during init (shared libs,
/// fonts, the user profile) before a document is even loaded.
constexpr int MinRequiredOpenFiles = 4096;

/// The number of bytes in count units of unitBytes each, for a unitBytes of at most 2^32. count
/// is a decimal number. Zero or a negative count gives zero, which stands for unlimited. A result
/// that rlim_t can't hold becomes the largest finite limit, RLIM_INFINITY - 1.
std::uint64_t limitInBytes(std::string_view count, std::uint64_t unitBytes);

/// Handles setconfig command with limit_... subcommands.
/// Returns true iff it handled the command, regardless of success/failure.
bool handleSetrlimitCommand(const StringVector& tokens);
}; // namespace Rlimit

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
