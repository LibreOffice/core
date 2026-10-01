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

namespace ServerPrivateInfo
{
inline constexpr std::string_view GroupName = "serverprivateinfo";
inline constexpr std::string_view FileName = "serverprivateinfo.json";
inline constexpr std::string_view FilePath =
    "/settings/systemconfig/serverprivateinfo/serverprivateinfo.json";

// Every key the file holds.
inline constexpr std::string_view Fields[] = {
    "ESignatureBaseUrl",
    "ESignatureClientId",
    "ESignatureSecret",
};

inline constexpr std::string_view OverrideFields[] = {
    "ESignatureClientId",
    "ESignatureSecret",
};

// The fields whose value is a credential, a subset of Fields.
inline constexpr std::string_view SecretFields[] = {
    "ESignatureSecret",
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
