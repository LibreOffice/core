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
#include <cassert>
#include <cstdint>
#include <limits>

#include <comphelper/legacyunoapinotice.hxx>

namespace
{
std::atomic<bool> g_legacyUnoApiUseFlag{ false };

// Number of LegacyApiWarningSuppression objects alive on this thread; while it is non-zero,
// notifyLegacyUnoApiUse does not raise the flag:
thread_local std::uint64_t g_legacyApiWarningSuppressions = 0;
}

bool comphelper::isLegacyUnoApi(std::u16string_view id) {
    return id.starts_with(u"com.sun.star.") || id.starts_with(u"cpo.");
}

void comphelper::notifyLegacyUnoApiUse(std::u16string_view id)
{
    if (g_legacyApiWarningSuppressions == 0 && isLegacyUnoApi(id))
    {
        g_legacyUnoApiUseFlag.store(true, std::memory_order_relaxed);
    }
}

bool comphelper::takeLegacyUnoApiUseFlag()
{
    return g_legacyUnoApiUseFlag.exchange(false, std::memory_order_relaxed);
}

comphelper::LegacyApiWarningSuppression::LegacyApiWarningSuppression()
{
    assert(g_legacyApiWarningSuppressions != std::numeric_limits<std::uint64_t>::max());
    ++g_legacyApiWarningSuppressions;
}

comphelper::LegacyApiWarningSuppression::~LegacyApiWarningSuppression()
{
    assert(m_aThread == std::this_thread::get_id());
    assert(g_legacyApiWarningSuppressions > 0);
    --g_legacyApiWarningSuppressions;
}

bool comphelper::isLegacyApiWarningSuppressed() { return g_legacyApiWarningSuppressions > 0; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
