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
#include <docmodel/theme/RectangleAlignment.hxx>
#include <basegfx/units/Length.hxx>
#include <tools/degree.hxx>

namespace model
{
/// A copy of the shape that fades out, scaled and skewed, and then moved by the distance in the
/// direction.
class DOCMODEL_DLLPUBLIC ReflectionEffect
{
public:
    gfx::Length maBlurRadius = 0_emu;
    /// Distance of the reflection from the shape, in the direction.
    gfx::Length maDistance = 0_emu;
    /// Clockwise from the positive x axis.
    Degree60000 mnDirection = 0_deg60000;
    /// In 1/1000 percent. A negative scale mirrors the reflection. The reflection presets have a
    /// vertical scale of -100%, which is a mirror image below the shape.
    sal_Int32 mnScaleX = 100000;
    sal_Int32 mnScaleY = 100000;
    Degree60000 mnSkewX = 0_deg60000;
    Degree60000 mnSkewY = 0_deg60000;
    /// The point of the shape that the scale and the skew are relative to.
    RectangleAlignment meAlignment = RectangleAlignment::Bottom;
    bool mbRotateWithShape = true;
    /// Opacity of the reflection where its fade starts and ends, and the positions of the start
    /// and the end from the edge next to the shape, all in 1/1000 percent.
    sal_Int32 mnStartAlpha = 100000;
    sal_Int32 mnStartPosition = 0;
    sal_Int32 mnEndAlpha = 0;
    sal_Int32 mnEndPosition = 100000;
    /// Clockwise from the positive x axis.
    Degree60000 mnFadeDirection = 5400000_deg60000;

    bool operator==(const ReflectionEffect&) const = default;
};

} // end of namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
