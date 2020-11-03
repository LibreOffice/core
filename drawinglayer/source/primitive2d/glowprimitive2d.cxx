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

#include <drawinglayer/primitive2d/glowprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
GlowPrimitive2D::GlowPrimitive2D(const Color& rGlowColor, double fRadius,
                                 const Primitive2DContainer& rChildren)
    : GroupPrimitive2D(rChildren)
    , maGlowColor(rGlowColor)
    , mfGlowRadius(fRadius)
{
}

bool GlowPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BasePrimitive2D::operator==(rPrimitive))
    {
        const GlowPrimitive2D& rCompare = static_cast<const GlowPrimitive2D&>(rPrimitive);

        return (getGlowRadius() == rCompare.getGlowRadius()
                && getGlowColor() == rCompare.getGlowColor());
    }

    return false;
}

basegfx::B2DRange
GlowPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
{
    basegfx::B2DRange aRetval(GroupPrimitive2D::getB2DRange(rViewInformation));
    // We need additional space for the glow from all sides
    aRetval.grow(getGlowRadius());
    return aRetval;
}

// provide unique ID
ImplPrimitive2DIDBlock(GlowPrimitive2D, PRIMITIVE2D_ID_GLOWPRIMITIVE2D)

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
