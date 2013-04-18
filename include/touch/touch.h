/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2013 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TOUCH_TOUCH_H
#define INCLUDED_TOUCH_TOUCH_H

#include <config_features.h>

#if !HAVE_FEATURE_DESKTOP

// Functions to be implemented by the app-specifc upper or less
// app-specific but platform-specific medium layer on touch-based
// platforms. The same API is used on each such platform. There are
// called from low level LibreOffice code. Note that these are just
// declared here in this header in the "touch" module, the
// per-platform implementations are elsewhere.

#ifdef __cplusplus
extern "C" {
#endif

void lo_show_keyboard();
void lo_hide_keyboard();

// Functions to be implemented in the medium platform-specific layer
// to be called from the app-specific UI layer.

void lo_keyboard_did_hide();

#ifdef __cplusplus
}
#endif

#endif // HAVE_FEATURE_DESKTOP

#endif // INCLUDED_TOUCH_TOUCH_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
