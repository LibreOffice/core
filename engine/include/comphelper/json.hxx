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

#include <comphelper/comphelperdllapi.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>

namespace com::sun::star::uno
{
class Any;
class Type;
}

// Helpers to convert UNO values to JSON and back.

namespace comphelper {

// Append a JSON serialisation of the UNO value at `value` (typed `type`) to `buf`; interface
// values render as `null`, enums by their IDL name as a JSON string:
COMPHELPER_DLLPUBLIC
void appendUnoAsJson(OStringBuffer& buf, cpo::uno::Type const& type, void const* value);

// Decode the JSON value `json` into an `Any` of type `type`; throws css::uno::RuntimeException
// on malformed input or on a type whose decoder is not implemented yet:
COMPHELPER_DLLPUBLIC
cpo::uno::Any parseJsonToAny(OUString const& json, cpo::uno::Type const& type);

// Decode the JSON value `json` into an `Any` of inferred type (`VOID`, `BOOLEAN`, `DOUBLE`,
// `STRING`, or sequence of `ANY`); throws css::uno::RuntimeException on malformed input and on JSON
// object representations:
COMPHELPER_DLLPUBLIC cpo::uno::Any parseJsonToInferredAny(OUString const & json);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
