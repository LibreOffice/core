/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Dummy implementations of the callback functions in the UI layer
// that the LO layer calls. As this experimental Android app doesn't
// handle any of that, these do nothing.

#include <touch/touch.h>

extern "C"
__attribute__ ((visibility("default")))
void
touch_ui_selection_start(MLOSelectionKind kind,
                         const void *documentHandle,
                         MLORect *rectangles,
                         int rectangleCount,
                         void *preview)
{
}

extern "C"
__attribute__ ((visibility("default")))
void
touch_ui_selection_resize_done(bool success,
                               const void *documentHandle,
                               MLORect *rectangles,
                               int rectangleCount)
{
}

extern "C"
__attribute__ ((visibility("default")))
void
touch_ui_selection_none()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
