/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#if defined(EMSCRIPTEN)

#ifdef __cplusplus
extern "C" {
#endif

const char *lo_get_app_data_dir(void);

#ifdef __cplusplus
}
#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
