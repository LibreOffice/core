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

// Functions to be implemented by the upper/medium layers on
// non-desktop touch-based platforms, with the same API on each such
// platform. Note that these are just declared here in this header in
// the "touch" module, the per-platform implementations are elsewhere.

#ifdef __cplusplus
extern "C" {
#endif

void lo_show_keyboard();
void lo_hide_keyboard();

#ifdef __cplusplus
}
#endif

#endif // HAVE_FEATURE_DESKTOP

#endif // INCLUDED_TOUCH_TOUCH_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
