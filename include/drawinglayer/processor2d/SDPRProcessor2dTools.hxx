/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <sal/config.h>

namespace drawinglayer::primitive2d
{
class FillGraphicPrimitive2D;
}

namespace drawinglayer::geometry
{
class ViewInformation2D;
}

namespace basegfx
{
class B2DRange;
}

class Bitmap;
class BitmapEx;

namespace drawinglayer::processor2d
{
/** helper to process FillGraphicPrimitive2D:

        In places that want to implement direct rendering of this primitive
        e.g. in SDPRs all impls would need to handle the FillGraphicAttribute
        settings and the type of Graphic. Unify this by this helper in one place
        since this may get complicated (many cases to cover).
        It will create and return a BitmapEx when direct tiled rendering is
        preferable and suggested.
        Of course every impl may still do what it wants, this is just to make
        implementations easier.

        @param rFillGraphicPrimitive2D
        The primitive to work on

        @param rViewInformation2D
        The ViewInformation to work with (from the processor)

        @param rTarget
        The prepared PixelData to use for tiled rendering. If this
        is empty on return this means to evtl. use the decompose.
        Please hand in an empty one to make this work.

        @param rFillUnitRange
        This is a modifiable copy of FillGraphicAttribute.getGraphicRange(). We
        need a modifiable one since params since OffsetX/OffsetY in
        FillGraphicAttribute may require to change/adapt this if used

        @param fBigDiscreteArea
        Defines starting with which number of square pixels a target is seen
        to be painted 'big'

        @return
        false: rendering is not needed (invalid, outside, ...), done
        true: rendering is needed
           -> if rTarget is filled, use for tiled rendering
           -> if not, use fallback (primitive decomposition)

        For the various reasons/things that get checked/tested/done, please
        see the implementation
    */
bool prepareBitmapForDirectRender(
    const drawinglayer::primitive2d::FillGraphicPrimitive2D& rFillGraphicPrimitive2D,
    const drawinglayer::geometry::ViewInformation2D& rViewInformation2D, Bitmap& rTarget,
    basegfx::B2DRange& rFillUnitRange, double fBigDiscreteArea = 300.0 * 300.0);

/** helper to react/process if OffsetX/OffsetY of the FillGraphicAttribute is used.

        This is old but hard to remove stuff that allows hor/ver offset when
        tiled fill is used. To solve that, create pixel data that doubles
        resp. in width/height and copies the off-setted version of the bitmap
        information to the extra space, so rendering does not need to do that.

        Since this doubles the geometry, an adaption of the used fill range
        (here rFillUnitRange in unit coordinates) also needs to be adapted,
        refer to usage.
    */
void takeCareOfOffsetXY(
    const drawinglayer::primitive2d::FillGraphicPrimitive2D& rFillGraphicPrimitive2D,
    Bitmap& rTarget, basegfx::B2DRange& rFillUnitRange);

/** helper to calculate a discrete visible range based on a given logic range
    and a current ViewInformation2D. This is used for pixel renderers.
    It is handy to check for visibility of a primitive, but also to continue
    working with just the visible discrete part.

    The result rDiscreteVisibleRange will be reset if rContentRange is empty.
    Else it will be calculated. If ViewInformation2D does have a valid
    Viewport (getDiscreteViewport is not empty), it will also be clipped
    against that to calculate the visible part.

        @param rDiscreteVisibleRange
        The logic range in view-coordinates (will be transformed by
        getObjectToViewTransformation)

        @param rContentRange
        The logic input range in view-coordinates (will be transformed by
        getObjectToViewTransformation)

        @param rViewInformation2D
        The ViewInformation to work with (from the processor)
    */
void calculateDiscreteVisibleRange(
    basegfx::B2DRange& rDiscreteVisibleRange, const basegfx::B2DRange& rContentRange,
    const drawinglayer::geometry::ViewInformation2D& rViewInformation2D);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
