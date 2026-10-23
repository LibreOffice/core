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

#include <common/VersionStore.hpp>

#include <string>
#include <vector>

class Prefs;

/// Version store settings, kept in Prefs under "versions.*".
namespace versioning
{
/// The limits @prefs ask for, each cap clamped to at least 1. Never writes back.
Limits limitsFromPrefs(const Prefs& prefs);

/// Root the store at @appDataDir/versions with the limits from @prefs.
/// Nothing under @appDataDir or @extraExcludedPrefixes is versioned.
void initializeForApp(const std::string& appDataDir, const Prefs& prefs,
                      std::vector<std::string> extraExcludedPrefixes,
                      const std::string& exportDirectory);

/// Reply to VERSIONSTORESTATUS: the limits in force and the store size, as JSON.
std::string statusJson();

/// Handle SETVERSIONSTORELIMITS: store the fields present in @payload in
/// @prefs, then apply the resulting limits. A malformed payload changes nothing.
void applyLimitsJson(const std::string& payload, Prefs& prefs);

/// Handle CLEARALLVERSIONS: delete every version, return the size left.
std::string clearAllJson();
} // namespace versioning

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
