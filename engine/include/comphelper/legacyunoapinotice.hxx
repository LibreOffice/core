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

#include <cassert>
#include <string_view>
#include <thread>

#include "comphelperdllapi.h"

namespace comphelper
{
COMPHELPER_DLLPUBLIC bool isLegacyUnoApi(std::u16string_view id);

COMPHELPER_DLLPUBLIC void notifyLegacyUnoApiUse(std::u16string_view id);

COMPHELPER_DLLPUBLIC bool takeLegacyUnoApiUseFlag();

/** Keeps notifyLegacyUnoApiUse quiet on the thread that created it, for as long as it lives; nests,
    and copying opens one more scope. A script host that lets script code open such a scope owns the
    objects, so that an unbalanced one ends with the script rather than with the thread. */
class COMPHELPER_DLLPUBLIC LegacyApiWarningSuppression
{
public:
    LegacyApiWarningSuppression();
    LegacyApiWarningSuppression([[maybe_unused]] LegacyApiWarningSuppression const& other)
        : LegacyApiWarningSuppression()
    {
        assert(other.m_aThread == m_aThread);
    }
    ~LegacyApiWarningSuppression();

    LegacyApiWarningSuppression& operator=(LegacyApiWarningSuppression const&) = delete;

private:
#ifndef NDEBUG
    // The depth an object counts in belongs to one thread, so opening and closing a scope must
    // happen on the same one:
    std::thread::id m_aThread = std::this_thread::get_id();
#endif
};

/** Suppresses notifyLegacyUnoApiUse for script code that ships with the office (the pyuno
    bootstrap, a registered component's module) rather than being embedded in a document. */
[[nodiscard]] inline LegacyApiWarningSuppression suppressLegacyApiWarning() { return {}; }

COMPHELPER_DLLPUBLIC bool isLegacyApiWarningSuppressed();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
