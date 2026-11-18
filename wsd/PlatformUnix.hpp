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
 * Unix platform initialization and capabilities.
 * Functions: dropCapability(), symlinkPathToJail()
 */

#pragma once

// macOS can be both server and mobile, so let's include it here, too
#if defined(MACOS)
#include "macos.h"
#endif

#ifdef __linux__

#if !MOBILEAPP
#include <common/security.h>
#include <sys/inotify.h>
#endif

#endif
