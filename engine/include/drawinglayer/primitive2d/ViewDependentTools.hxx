/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>

namespace drawinglayer::primitive2d
{
/** Calculate discrete sizes and unit visible range for a given object transformation.
    Transforms the unit range to get the discrete range, clips against the viewport,
    and computes the relative visible portion as unit coordinates [0..1].
 */
void DRAWINGLAYERCORE_DLLPUBLIC calculateDiscreteVisibleRanges(
    const geometry::ViewInformation2D& rViewInformation,
    const basegfx::B2DHomMatrix& rObjectTransformation, basegfx::B2DRange& rDiscreteRange,
    basegfx::B2DRange& rVisibleDiscreteRange, basegfx::B2DRange& rUnitVisibleRange);

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
