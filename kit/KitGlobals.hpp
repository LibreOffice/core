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

#if !MOBILEAPP

#include <kit/BgSaveWatchDog.hpp>

std::unique_ptr<BackgroundSaveWatchdog> BackgroundSaveWatchdog::Instance;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
