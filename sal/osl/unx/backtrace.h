/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SAL_OSL_UNX_BACKTRACE_H
#define INCLUDED_SAL_OSL_UNX_BACKTRACE_H

#include <config_features.h>

#if HAVE_FEATURE_BACKTRACE /* GNU backtrace implementation available */

#include <execinfo.h>

#else

#ifdef __cplusplus
extern "C" {
#endif

/* backtrace function with same behaviour as defined in GNU libc */

int backtrace( void **buffer, int max_frames );

char ** backtrace_symbols(void * const * buffer, int size);

void backtrace_symbols_fd( void **buffer, int size, int fd );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
