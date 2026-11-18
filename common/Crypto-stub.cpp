/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Stub implementation of cryptographic support key validation.
 * Classes: SupportKey - Returns no-op/always-valid responses
 */

#include <config.h>

#include "Crypto.hpp"

#include <Poco/DateTime.h>

struct SupportKeyImpl {};

SupportKey::SupportKey([[maybe_unused]] const std::string& key) {}

SupportKey::~SupportKey() {}

bool SupportKey::verify() { return true; }

int SupportKey::validDaysRemaining() { return 0; }

Poco::DateTime SupportKey::expiry() const { return Poco::DateTime(); }

std::string SupportKey::data() const { return std::string(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
