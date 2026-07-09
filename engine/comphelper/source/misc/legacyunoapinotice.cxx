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

#include <sal/config.h>

#include <atomic>

#include <comphelper/legacyunoapinotice.hxx>

namespace
{
std::atomic<bool> g_legacyUnoApiUseFlag{ false };
}

bool comphelper::isLegacyUnoApi(std::u16string_view id) { return id.starts_with(u"com.sun.star."); }

void comphelper::notifyLegacyUnoApiUse(std::u16string_view id)
{
    if (isLegacyUnoApi(id))
    {
        g_legacyUnoApiUseFlag.store(true, std::memory_order_relaxed);
    }
}

bool comphelper::takeLegacyUnoApiUseFlag()
{
    return g_legacyUnoApiUseFlag.exchange(false, std::memory_order_relaxed);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
