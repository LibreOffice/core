/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SAL_INTERNAL_RTLLIFECYCLE_H
#define INCLUDED_SAL_INTERNAL_RTLLIFECYCLE_H

#if defined __cplusplus
extern "C" {
#endif

void rtl_arena_init();

void rtl_arena_fini();

void ensureArenaSingleton();

void rtl_cache_init();

void rtl_cache_fini();

void ensureCacheSingleton();

void rtl_memory_init();

void rtl_memory_fini();

void ensureMemorySingleton();

void rtl_locale_init();

void rtl_locale_fini();

void ensureLocaleSingleton();

#if defined __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
