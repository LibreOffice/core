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

#include <docmodel/dllapi.h>
#include <docmodel/effect/ReflectionEffect.hxx>
#include <rtl/string.hxx>

namespace tools
{
class JsonWriter;
}

/* The reflection as a JSON object with one member per value. Lengths are in EMU, and the other
   values are in the units of model::ReflectionEffect.
 */

namespace model::effect
{
DOCMODEL_DLLPUBLIC OString convertToJSON(model::ReflectionEffect const& rEffect);

/// Writes the members of the reflection into the current object of rTree.
DOCMODEL_DLLPUBLIC void convertToJSONTree(tools::JsonWriter& rTree,
                                          model::ReflectionEffect const& rEffect);

/* Reads the reflection from JSON. A member that the JSON lacks keeps its value in rEffect. Gives
   false, and leaves rEffect as it was, for JSON that does not parse or has a value of the wrong
   type. */
DOCMODEL_DLLPUBLIC bool convertFromJSON(OString const& rJSON, model::ReflectionEffect& rEffect);

} // end of namespace model::effect

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
