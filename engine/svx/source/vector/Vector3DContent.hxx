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

#include <svx/VectorObjectContent.hxx>

class E3dObject;

namespace tools
{
class JsonWriter;
}

namespace svx::vector
{
/// Fills in the 3D part of what a 3D object or scene says about itself: the camera, the look and
/// the lights of the outermost scene, the 3D mapping and the 3D primitive of what is inside it.
void describeIn3D(const E3dObject& rObject, ObjectContent& rContent);

/// Writes the 3D part of the content into the open entry, nothing where it has none.
void writeIn3D(tools::JsonWriter& rWriter, const ObjectContent& rContent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
