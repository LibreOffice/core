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

#include <string_view>

namespace ViewSettings
{
// Fields in viewsetting.json that hold a user secret. These are never sent to
// the browser in cleartext and are preserved across a settings save unless the
// user replaces them. Listed once here so the server's redact-on-read and
// keep-on-save paths use the same set of fields.
inline constexpr std::string_view SecretFields[] = {
    "aiProviderAPIKey",
    "aiImageProviderAPIKey",
    "zoteroAPIKey",
    "signatureKey",
};

// Suffix appended to a secret field name to form its companion flag. Sent to
// the browser as true when a value is stored, so the dialog can show that a
// secret exists without receiving it. Sent back as true when the user left the
// field untouched, which asks the server to keep the stored value. For
// aiProviderAPIKey the flag is aiProviderAPIKeyStored. The flag is transport
// only and is removed before the file is written back.
inline constexpr std::string_view StoredFlagSuffix = "Stored";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
