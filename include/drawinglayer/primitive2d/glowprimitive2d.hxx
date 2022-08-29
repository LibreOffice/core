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

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <tools/color.hxx>

namespace drawinglayer::primitive2d
{
class DRAWINGLAYER_DLLPUBLIC GlowPrimitive2D final : public GroupPrimitive2D
{
private:
    /// a sequence used for buffering the last create2DDecomposition() result
    Primitive2DContainer maBuffered2DDecomposition;

    /// the Glow color to which all geometry is to be forced; includes alpha
    Color maGlowColor;

    /// the Glow size, in logical units (100ths of mm)
    double mfGlowRadius;

    /// last used DiscreteGlowRadius and ClippedRange
    double mfLastDiscreteGlowRadius;
    basegfx::B2DRange maLastClippedRange;

    /// helpers
    bool prepareValuesAndcheckValidity(basegfx::B2DRange& rRange, basegfx::B2DRange& rClippedRange,
                                       basegfx::B2DVector& rDiscreteSize,
                                       double& rfDiscreteGlowRadius,
                                       const geometry::ViewInformation2D& rViewInformation) const;

protected:
    /** access methods to maBuffered2DDecomposition, same as in
     *  BufferedDecompositionPrimitive2D
     */
    const Primitive2DContainer& getBuffered2DDecomposition() const
    {
        return maBuffered2DDecomposition;
    }
    void setBuffered2DDecomposition(Primitive2DContainer&& rNew)
    {
        maBuffered2DDecomposition = std::move(rNew);
    }

    /** method which is to be used to implement the local decomposition of a 2D primitive. */
    virtual void create2DDecomposition(Primitive2DContainer& rContainer,
                                       const geometry::ViewInformation2D& rViewInformation) const;

public:
    /// constructor
    GlowPrimitive2D(const Color& rGlowColor, double fRadius, Primitive2DContainer&& rChildren);

    /// data read access
    const Color& getGlowColor() const { return maGlowColor; }
    double getGlowRadius() const { return mfGlowRadius; }

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /// get range
    virtual basegfx::B2DRange
    getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

    /// The default implementation will return an empty sequence
    virtual void
    get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor,
                       const geometry::ViewInformation2D& rViewInformation) const override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;
};
} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
