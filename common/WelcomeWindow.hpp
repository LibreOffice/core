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

#include <algorithm>
#include <utility>

namespace Desktop
{
// The width and height of the window that plays the welcome slideshow, for a work area of the given
// width. All three are in device-independent pixels. The window is 16:9, as the slideshow is, and
// 40% of the width of the work area, but at least 800x450 and at most 1280x720.
inline std::pair<int, int> welcomeWindowSize(int workAreaWidth)
{
    const int width = std::clamp(static_cast<int>(workAreaWidth * 0.4), 800, 1280);
    const int height = std::clamp(static_cast<int>((width * 9.0) / 16.0), 450, 720);
    return { width, height };
}
} // namespace Desktop

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
