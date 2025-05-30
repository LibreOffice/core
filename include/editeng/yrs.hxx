/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

extern "C" {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextern-c-compat"
#endif
#include <libyrs.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
}

// check input is valid values to find encoding bugs early
#define yvalidate(cond)                                                                            \
    if (!(cond))                                                                                   \
    {                                                                                              \
        std::abort();                                                                              \
    }

struct YOutputDeleter
{
    void operator()(YOutput* const p) const { youtput_destroy(p); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
