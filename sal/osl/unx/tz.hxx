/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <cstdint>

namespace osl::tz
{
struct BrokenDown
{
    int year, month, day, hour, minute, second, dayOfWeek;
};

/// UTC epoch seconds -> broken-down UTC. Pure math (no tz data). Replaces gmtime_r.
bool epochToUtc(std::int64_t epochSec, BrokenDown& out);

/// Broken-down UTC -> epoch seconds. Pure math (no tz data). Replaces timegm.
bool utcToEpoch(const BrokenDown& bd, std::int64_t& outEpoch);

/// UTC offset in seconds for a UTC timestamp. Positive = east of Greenwich.
/// Uses ICU ucal_* API with default timezone. Returns 0 on error (UTC fallback).
std::int32_t getUtcOffsetForUtcTime(std::int64_t utcEpochSec);

/// UTC offset in seconds for a local timestamp (iterative refinement for DST).
std::int32_t getUtcOffsetForLocalTime(std::int64_t localEpochSec);

} // namespace osl::tz

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
