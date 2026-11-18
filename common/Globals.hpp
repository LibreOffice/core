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

#include <common/Anonymizer.hpp>
#include <common/Log.hpp>
#include <common/StaticLogHelper.hpp>

// Order of construction is unspecified when static objects are defined in different translation units, so
// put globals here and include this once to specify order of construction/destruction.

namespace Log {
    /// Helper to avoid destruction ordering issues.
    StaticHelper Static;

    StaticUIHelper StaticUILog;

    thread_local GenericLogger* StaticHelper::_threadLocalLogger = nullptr;

    thread_local Prefix Prefix::Instance;
}

std::unique_ptr<Anonymizer> Anonymizer::_instance;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
