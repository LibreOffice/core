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

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

namespace drawinglayer::primitive2d
{
/** TextEffectStyle2D definition */
enum class TextEffectStyle2D
{
    ReliefEmbossedDefault,
    ReliefEngravedDefault,
    ReliefEmbossed,
    ReliefEngraved,
    Outline
};

/** TextEffectPrimitive2D class

    This primitive embeds text primitives (normally, as can be seen can
    also be used for any other primitives) which have some TextEffect applied
    and create the needed geometry and embedding on decomposition.
*/
class TextEffectPrimitive2D final : public BufferedDecompositionPrimitive2D
{
private:
    /// the text (or other) content
    Primitive2DContainer maTextContent;

    /// the style to apply, the direction and the rotation center
    const basegfx::B2DPoint maRotationCenter;
    double mfDirection;
    TextEffectStyle2D meTextEffectStyle2D;

    /** the last used object to view transformtion used from getDecomposition
        for decide buffering
     */
    basegfx::B2DHomMatrix maLastObjectToViewTransformation;

    /// create local decomposition
    virtual void
    create2DDecomposition(Primitive2DContainer& rContainer,
                          const geometry::ViewInformation2D& rViewInformation) const override;

public:
    /// constructor
    TextEffectPrimitive2D(const Primitive2DContainer& rTextContent,
                          const basegfx::B2DPoint& rRotationCenter, double fDirection,
                          TextEffectStyle2D eTextEffectStyle2D);

    /// data read access
    const Primitive2DContainer& getTextContent() const { return maTextContent; }
    const basegfx::B2DPoint& getRotationCenter() const { return maRotationCenter; }
    double getDirection() const { return mfDirection; }
    TextEffectStyle2D getTextEffectStyle2D() const { return meTextEffectStyle2D; }

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /** own get range implementation to solve more effective. Content is by definition displaced
        by a fixed discrete unit, thus the contained geometry needs only once be asked for its
        own basegfx::B2DRange
     */
    virtual basegfx::B2DRange
    getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;

    /// Override standard getDecomposition to be view-dependent here
    virtual void
    get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor,
                       const geometry::ViewInformation2D& rViewInformation) const override;
};

} // end of namespace primitive2d::drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
