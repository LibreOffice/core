/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

// On a Wayland or X11 session, installs an application-wide event filter that repairs bitmap
// mouse cursors so they are drawn at the right size and position on scaled displays. Does
// nothing on other display servers. Call once after the QApplication has been created.
void installBitmapCursorFix();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
