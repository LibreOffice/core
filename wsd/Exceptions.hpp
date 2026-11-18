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

// Exception classes to differentiate between the
// different error situations and handling.

#pragma once

#include <atomic>
#include <stdexcept>
#include <string>

// not beautiful
#define EXCEPTION_DECL(type,parent_cl)  \
    class type : public parent_cl \
    { \
    public: \
        static std::atomic<size_t> count; \
        type(const std::string &str) : parent_cl(str) \
            { type::count++; } \
    };

// Generic COOL errors and base for others.
class CoolException : public std::runtime_error
{
public:
    std::string toString() const
    {
        return what();
    }
protected:
    using std::runtime_error::runtime_error;
};

EXCEPTION_DECL(StorageSpaceLowException,CoolException)

/// General exception thrown when we are not able to
/// connect to storage.
EXCEPTION_DECL(StorageConnectionException,CoolException)

/// A bad-request exception that is meant to signify,
/// and translate into, an HTTP bad request.
EXCEPTION_DECL(BadRequestException,CoolException)

/// A bad-argument exception that is meant to signify,
/// and translate into, an HTTP bad request.
EXCEPTION_DECL(BadArgumentException,BadRequestException)

/// An authorization exception that is meant to signify,
/// and translate into, an HTTP unauthorized error.
EXCEPTION_DECL(UnauthorizedRequestException,CoolException)

/// A service-unavailable exception that is meant to signify
/// an internal error.
EXCEPTION_DECL(ServiceUnavailableException,CoolException)

/// Badly formed data we are parsing
EXCEPTION_DECL(ParseError,CoolException)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
