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

#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/softedgeprimitive2d.hxx>

namespace drawinglayer::primitive2d
{
SoftEdgePrimitive2D::SoftEdgePrimitive2D(double fRadius, Primitive2DContainer&& aChildren)
    : GroupPrimitive2D(std::move(aChildren))
    , mfRadius(fRadius)
{
}

bool SoftEdgePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (GroupPrimitive2D::operator==(rPrimitive))
    {
        auto& rCompare = static_cast<const SoftEdgePrimitive2D&>(rPrimitive);
        return getRadius() == rCompare.getRadius();
    }

    return false;
}

void SoftEdgePrimitive2D::get2DDecomposition(
    Primitive2DDecompositionVisitor& rVisitor,
    const geometry::ViewInformation2D& rViewInformation) const
{
    if (getChildren().empty())
        return;

    if (!mbInMaskGeneration)
    {
        GroupPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
        return;
    }

    // create a modifiedColorPrimitive containing the *black* color and the content. Using black
    // on white allows creating useful mask in VclPixelProcessor2D::processSoftEdgePrimitive2D.
    basegfx::BColorModifierSharedPtr aBColorModifier
        = std::make_shared<basegfx::BColorModifier_replace>(basegfx::BColor());

    const Primitive2DReference xRef(
        new ModifiedColorPrimitive2D(Primitive2DContainer(getChildren()), aBColorModifier));
    rVisitor.visit(xRef);
}

sal_uInt32 SoftEdgePrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_SOFTEDGEPRIMITIVE2D;
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
