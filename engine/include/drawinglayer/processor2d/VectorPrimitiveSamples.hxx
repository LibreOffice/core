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

#include <drawinglayer/drawinglayerdllapi.h>

#include <string_view>

namespace tools
{
class JsonWriter;
}

namespace drawinglayer
{
/// Writes the sample that rName names to rWriter as the array "primitives", in the JSON that
/// Primitive2dJsonProcessor writes for an editing view. A sample is a small primitive tree built
/// in code, one or a few for each primitive type the processor writes, such as "backgroundColor",
/// "graphicCrop" or "textDecoratedPortion". The images of a sample travel by checksum alone and
/// are not kept. Returns false, with nothing written, when no sample has that name.
DRAWINGLAYER_DLLPUBLIC bool writeVectorPrimitiveSample(tools::JsonWriter& rWriter,
                                                       std::string_view rName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
