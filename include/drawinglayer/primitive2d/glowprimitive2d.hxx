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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GLOWPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GLOWPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/color/bcolor.hxx>

namespace drawinglayer
{
namespace primitive2d
{
class DRAWINGLAYER_DLLPUBLIC GlowPrimitive2D final : public GroupPrimitive2D
{
private:
    /// the Glow transformation, normally just an offset
    basegfx::B2DHomMatrix maGlowTransform;

    /// the Glow color to which all geometry is to be forced
    basegfx::BColor maGlowColor;

public:
    /// constructor
    GlowPrimitive2D(const basegfx::B2DHomMatrix& rGlowTransform, const basegfx::BColor& rGlowColor,
                    const Primitive2DContainer& rChildren);

    /// data read access
    const basegfx::B2DHomMatrix& getGlowTransform() const { return maGlowTransform; }
    const basegfx::BColor& getGlowColor() const { return maGlowColor; }

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /// get range
    virtual basegfx::B2DRange
    getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

    ///  create decomposition
    virtual void
    get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor,
                       const geometry::ViewInformation2D& rViewInformation) const override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;
};
} // end of namespace primitive2d
} // end of namespace drawinglayer

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GLOWPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
