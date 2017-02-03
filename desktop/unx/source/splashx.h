/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_DESKTOP_UNX_SOURCE_SPLASHX_H
#define INCLUDED_DESKTOP_UNX_SOURCE_SPLASHX_H

#include <rtl/ustring.h>

#ifdef __cplusplus
extern "C" {
#endif

struct splash;

struct splash* splash_create(rtl_uString* pAppPath, int argc, char** argv);

void splash_destroy(struct splash* splash);

void splash_draw_progress(struct splash* splash, int progress);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // INCLUDED_DESKTOP_UNX_SOURCE_SPLASHX_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
