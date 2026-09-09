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
inline constexpr std::string_view FileName = "viewsetting.json";

// Fields in viewsetting.json that hold a user secret. These are never sent to
// the browser in cleartext and are preserved across a settings save unless the
// user replaces them.
inline constexpr std::string_view SecretFields[] = {
    "aiProviderAPIKey",
    "aiImageProviderAPIKey",
    "zoteroAPIKey",
    "signatureKey",
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
