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

#include <sal/config.h>

#include <cstdlib>
#include <iostream>
#include <string_view>

#include <jsuno/jsuno.hxx>
#include <rtl/string.hxx>

jsuno::Result testexec(OString const & script) {
    return jsuno::execute(
        std::u8string_view(u8"", 1),
        std::string_view(script.getStr(), script.getLength() + 1), "<input>", 1,
        [](std::string_view level, std::string_view message) {
            std::cout << "console." << level << ": " << message << std::endl;
            if (level == "assert") {
                std::abort();
            }
        },
        {});
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
