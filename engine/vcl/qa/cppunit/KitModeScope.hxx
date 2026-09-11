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

#include <comphelper/kit.hxx>

/// Turns kit mode on for as long as it is in scope and puts back the setting that was there
/// before. Only in kit mode does a data container write its bytes out to a temporary file.
class KitModeScope
{
    const bool mbWasActive;

public:
    explicit KitModeScope(bool bActive = true)
        : mbWasActive(comphelper::COKit::isActive())
    {
        comphelper::COKit::setActive(bActive);
    }

    ~KitModeScope() { comphelper::COKit::setActive(mbWasActive); }

    KitModeScope(const KitModeScope&) = delete;
    KitModeScope& operator=(const KitModeScope&) = delete;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
