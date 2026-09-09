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

#include <span>
#include <string_view>

#include <common/ServerPrivateInfo.hpp>
#include <common/ViewSettings.hpp>

// The settings files that hold a secret, and the field in each one that carries it. A settings
// file not named here holds no secret.
namespace SettingsSecrets
{
// Suffix appended to a secret field name to form its companion flag, so that aiProviderAPIKey
// pairs with aiProviderAPIKeyStored. The flag is true when the server holds a value for the field
// that it did not send down, and true on the way back when the field was left untouched and the
// stored value stays. It travels with the file but is not part of it.
inline constexpr std::string_view StoredFlagSuffix = "Stored";

/// The secret fields the named settings file holds. Empty for a file that holds none.
inline std::span<const std::string_view> fieldsFor(std::string_view fileName)
{
    if (fileName == ViewSettings::FileName)
        return ViewSettings::SecretFields;
    if (fileName == ServerPrivateInfo::FileName)
        return ServerPrivateInfo::SecretFields;
    return {};
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
