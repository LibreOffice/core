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

#pragma once

#include <string_view>

namespace LangUtil
{
    inline bool isRtlLanguage(std::string_view language)
    {
        // Compare only the language code, so that "arn" doesn't count as "ar".
        static constexpr std::string_view letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        const std::string_view code = language.substr(0, language.find_first_not_of(letters));

        if (code == "ar" ||
            code == "arc" ||
            code == "dv" ||
            code == "fa" ||
            code == "ha" ||
            code == "he" ||
            code == "khw" ||
            code == "ks" ||
            code == "ku" ||
            code == "ps" ||
            code == "ur" ||
            code == "yi")
            return true;

        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
