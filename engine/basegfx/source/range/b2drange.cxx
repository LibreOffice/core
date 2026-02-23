/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

namespace basegfx
{
    B2DRange::B2DRange(const B2IRange& rRange)
    {
        if (!rRange.isEmpty())
        {
            maRangeX = basegfx::BasicRange<ValueType, TraitsType>(rRange.getMinX());
            maRangeY = basegfx::BasicRange<ValueType, TraitsType>(rRange.getMinY());

            maRangeX.expand(rRange.getMaxX());
            maRangeY.expand(rRange.getMaxY());
        }
    }

    void B2DRange::transform(const B2DHomMatrix& rMatrix)
    {
        if(isEmpty() || rMatrix.isIdentity())
            return;

        // Compute the AABB of the transformed box directly from matrix
        // coefficients, instead of transforming all four corners and
        // expanding. For an affine transform the extremes of each output
        // coordinate are reached at corners chosen per coefficient: a
        // non-negative coefficient pairs the input min with the output min,
        // a negative coefficient pairs the input max with the output min.
        // Translation is the same constant offset for min and max, so it
        // is folded into the initial value below.
        // (Arvo, Graphics Gems I, 1990.)

        const double oldMinX = getMinX();
        const double oldMaxX = getMaxX();
        const double oldMinY = getMinY();
        const double oldMaxY = getMaxY();

        const double m00 = rMatrix.get(0, 0);
        const double m01 = rMatrix.get(0, 1);
        const double m10 = rMatrix.get(1, 0);
        const double m11 = rMatrix.get(1, 1);

        double newMinX = rMatrix.get(0, 2);
        double newMaxX = newMinX;
        double newMinY = rMatrix.get(1, 2);
        double newMaxY = newMinY;

        auto axis = [](const double matrixValue, const double inLow, const double inHigh, double& outLow, double& outHigh)
        {
            if (matrixValue >= 0.0)
            {
                outLow += matrixValue * inLow;
                outHigh += matrixValue * inHigh;
            }
            else
            {
                outLow += matrixValue * inHigh;
                outHigh += matrixValue * inLow;
            }
        };

        axis(m00, oldMinX, oldMaxX, newMinX, newMaxX);
        axis(m01, oldMinY, oldMaxY, newMinX, newMaxX);
        axis(m10, oldMinX, oldMaxX, newMinY, newMaxY);
        axis(m11, oldMinY, oldMaxY, newMinY, newMaxY);

        *this = B2DRange(newMinX, newMinY, newMaxX, newMaxY);
    }

    void B2DRange::translate(double fTranslateX, double fTranslateY)
    {
        if(!(fTools::equalZero(fTranslateX) && fTools::equalZero(fTranslateY)))
        {
            *this = basegfx::B2DRange(fTranslateX + getMinX(),
                                      fTranslateY + getMinY(),
                                      fTranslateX + getMaxX(),
                                      fTranslateY + getMaxY());
        }
    }

    B2DRange& B2DRange::operator*=(const basegfx::B2DHomMatrix& rMatrix)
    {
        transform(rMatrix);
        return *this;
    }

    const B2DRange& B2DRange::getUnitB2DRange()
    {
        static const B2DRange aUnitB2DRange(0.0, 0.0, 1.0, 1.0);

        return aUnitB2DRange;
    }

    B2IRange fround(const B2DRange& rRange)
    {
        return rRange.isEmpty() ?
            B2IRange() :
            B2IRange(fround(rRange.getMinimum()),
                     fround(rRange.getMaximum()));
    }

    B2DRange operator*( const ::basegfx::B2DHomMatrix& rMat, const B2DRange& rB2DRange )
    {
        B2DRange aRes( rB2DRange );
        aRes *= rMat;
        return aRes;
    }

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
