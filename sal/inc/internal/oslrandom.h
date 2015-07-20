/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SAL_INC_INTERNAL_OSLRANDOM_H
#define INCLUDED_SAL_INC_INTERNAL_OSLRANDOM_H

#if defined __cplusplus
extern "C" {
#endif

int osl_get_system_random_data(char* buffer, size_t desired_len);

#if defined __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
