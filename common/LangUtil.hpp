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

#include <string>

namespace LangUtil
{
    inline bool isRtlLanguage(const std::string& language)
    {
        if (language.rfind("ar", 0) == 0 ||
            language.rfind("arc", 0) == 0 ||
            language.rfind("dv", 0) == 0 ||
            language.rfind("fa", 0) == 0 ||
            language.rfind("ha", 0) == 0 ||
            language.rfind("he", 0) == 0 ||
            language.rfind("khw", 0) == 0 ||
            language.rfind("ks", 0) == 0 ||
            language.rfind("ku", 0) == 0 ||
            language.rfind("ps", 0) == 0 ||
            language.rfind("ur", 0) == 0 ||
            language.rfind("yi", 0) == 0)
            return true;

        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
