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

namespace model
{
/// A blurred band of the color around the outline of the shape, as wide as the radius.
class DOCMODEL_DLLPUBLIC GlowEffect
{
public:
    gfx::Length maRadius = 0_emu;
    /// The alpha transformation of the color is the opacity of the glow.
    ComplexColor maColor;

    bool operator==(const GlowEffect&) const = default;
};

} // end of namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
