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
#include <basegfx/units/Length.hxx>
#include <tools/degree.hxx>

namespace model
{
/// A blurred shadow in the color inside the outline of the shape, moved by the distance in the
/// direction.
class DOCMODEL_DLLPUBLIC InnerShadowEffect
{
public:
    gfx::Length maBlurRadius = 0_emu;
    /// Distance of the shadow from the shape, in the direction.
    gfx::Length maDistance = 0_emu;
    /// Clockwise from the positive x axis.
    Degree60000 mnDirection = 0_deg60000;
    /// The alpha transformation of the color is the opacity of the shadow.
    ComplexColor maColor;

    bool operator==(const InnerShadowEffect&) const = default;
};

} // end of namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
