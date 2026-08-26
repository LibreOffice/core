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

#include <jsuno/jsuno.hxx>
#include <rtl/ustring.hxx>

OUString testexec(OUString const & script) {
    return jsuno::execute(
        script, u"<input>"_ustr, 1,
        [](OUString const & level, OUString const & message) {
            std::cout << "console." << level << ": " << message << std::endl;
            if (level == u"assert") {
                std::abort();
            }
        },
        {}, nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
