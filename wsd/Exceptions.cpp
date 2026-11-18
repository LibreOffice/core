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

/*
 * Implementation of exception classes and counters.
 * Classes: CoolException
 */

#include <config.h>

#include "Exceptions.hpp"

#undef EXCEPTION_DECL

// not beautiful
#define EXCEPTION_DECL(type,unused) \
    std::atomic<size_t> type::count;

EXCEPTION_DECL(StorageSpaceLowException, CoolException)
EXCEPTION_DECL(StorageConnectionException, CoolException)
EXCEPTION_DECL(BadRequestException, CoolException)
EXCEPTION_DECL(BadArgumentException, BadRequestException)
EXCEPTION_DECL(UnauthorizedRequestException, CoolException)
EXCEPTION_DECL(ServiceUnavailableException, CoolException)
EXCEPTION_DECL(ParseError, CoolException)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
