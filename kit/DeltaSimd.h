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

// This is a C file by design.

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void simd_deltaInit(void);

int simd_initPixRowSimd(const uint32_t *from, uint32_t *scratch, size_t *scratchLen, uint64_t *rleMask);

int simd_copyRowSwapRB(unsigned char *dest, const unsigned char *src, unsigned int count);

#ifdef __cplusplus
} // extern "C"
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
