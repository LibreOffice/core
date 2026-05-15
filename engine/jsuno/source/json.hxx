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

#pragma once

#include <sal/config.h>

#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>

namespace com::sun::star::uno
{
class Any;
class Type;
}

// Internal-only helpers used by the JS-UNO proxy machinery to convert UNO values to JSON
// (notification payloads to the iframe) and back (per-call return values from the iframe).

// Append a JSON serialisation of the UNO value at `value` (typed `type`) to `buf`; interface
// values render as `null`, enums by their IDL name as a JSON string:
void appendUnoAsJson(OStringBuffer& buf, css::uno::Type const& type, void const* value);

// Decode the JSON value `json` into an `Any` of type `type`; throws css::uno::RuntimeException
// on malformed input or on a type whose decoder is not implemented yet:
css::uno::Any parseJsonToAny(OUString const& json, css::uno::Type const& type);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
