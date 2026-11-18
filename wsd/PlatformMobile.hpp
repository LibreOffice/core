/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Platform-specific code for mobile systems.
 * Functions: checkDiskSpaceOnRegisteredFileSystems()
 */

#pragma once

#if MOBILEAPP

#include <Kit.hpp>
#ifdef IOS
#include <ios.h>
#elif defined(MACOS)
#include <macos.h>
#elif defined(_WIN32)
#include <windows.hpp>
#elif defined(QTAPP)
#include <qt.hpp>
#elif defined(__ANDROID__)
#include <androidapp.hpp>
#elif WASMAPP
#include <wasmapp.hpp>
#endif

#endif // MOBILEAPP
