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
#include <docmodel/color/ComplexColor.hxx>
#include <docmodel/theme/RectangleAlignment.hxx>
#include <basegfx/units/Length.hxx>
#include <tools/degree.hxx>

namespace model
{
/// A blurred copy of the shape in the color behind the shape, scaled and skewed, and then moved by
/// the distance in the direction.
class DOCMODEL_DLLPUBLIC OuterShadowEffect
{
public:
    gfx::Length maBlurRadius = 0_emu;
    /// Distance of the shadow from the shape, in the direction.
    gfx::Length maDistance = 0_emu;
    /// Clockwise from the positive x axis.
    Degree60000 mnDirection = 0_deg60000;
    /// In 1/1000 percent. A negative scale mirrors the shadow.
    sal_Int32 mnScaleX = 100000;
    sal_Int32 mnScaleY = 100000;
    Degree60000 mnSkewX = 0_deg60000;
    Degree60000 mnSkewY = 0_deg60000;
    /// The point of the shape that the scale and the skew are relative to.
    RectangleAlignment meAlignment = RectangleAlignment::Bottom;
    bool mbRotateWithShape = true;
    /// The alpha transformation of the color is the opacity of the shadow.
    ComplexColor maColor;

    bool operator==(const OuterShadowEffect&) const = default;
};

} // end of namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
