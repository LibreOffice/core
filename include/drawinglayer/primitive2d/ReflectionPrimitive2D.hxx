/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/attribute/ReflectionAttribute.hxx>
#include <drawinglayer/primitive2d/BufferedDecompositionGroupPrimitive2D.hxx>

namespace drawinglayer::primitive2d
{
/** The reflection of the children: a vertically flipped copy of them, placed under them and
    more transparent the further it gets from them.

    The copy is flipped at the bottom edge of the range of the children and then moved down by the
    distance. The decomposition holds only the reflection, not the children themselves.
*/
class DRAWINGLAYER_DLLPUBLIC ReflectionPrimitive2D final
    : public BufferedDecompositionGroupPrimitive2D
{
private:
    attribute::ReflectionAttribute maReflection;

    /// The range of the children and the discrete blur radius of the buffered decomposition.
    /// A discrete blur radius below one pixel means the decomposition is not blurred.
    basegfx::B2DRange maLastContentRange;
    double mfLastDiscreteBlurRadius = 0.0;

protected:
    void create2DDecomposition(Primitive2DContainer& rContainer,
                               const geometry::ViewInformation2D& rViewInformation) const override;

public:
    ReflectionPrimitive2D(const attribute::ReflectionAttribute& rReflection,
                          Primitive2DContainer&& rChildren);

    const attribute::ReflectionAttribute& getReflection() const { return maReflection; }

    bool operator==(const BasePrimitive2D& rPrimitive) const override;

    basegfx::B2DRange
    getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

    void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor,
                            const geometry::ViewInformation2D& rViewInformation) const override;

    sal_uInt32 getPrimitive2DID() const override;
};
} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
