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

#include <scdllapi.h>

class Bitmap;
class Size;
class ScTableStyle;

// Render a table style's gallery swatch to a bitmap of the given pixel size. The
// colour derivation comes from ScComputeTableStyleSwatch, so a swatch drawn here
// and one drawn by the online client from the same values come out identical.
SC_DLLPUBLIC Bitmap ScRenderTableStyleSwatchBitmap(const ScTableStyle& rStyle,
                                                   const Size& rPixelSize);

// The swatch bitmap for the "no style" choice: the custom template in plain white.
SC_DLLPUBLIC Bitmap ScRenderNoneSwatchBitmap(const Size& rPixelSize);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
