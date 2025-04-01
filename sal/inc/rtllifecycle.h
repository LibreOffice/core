/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once


#if defined __cplusplus
extern "C" {
#endif

void rtl_arena_init(void);

void rtl_arena_fini(void);

void ensureArenaSingleton(void);

void rtl_cache_init(void);

void rtl_cache_fini(void);

void ensureCacheSingleton(void);

void rtl_locale_init(void);

void rtl_locale_fini(void);

void ensureLocaleSingleton(void);

#if defined __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
