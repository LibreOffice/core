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

#ifndef INCLUDED_BASEGFX_RANGE_B3DRANGE_HXX
#define INCLUDED_BASEGFX_RANGE_B3DRANGE_HXX

#include <basegfx/vector/b3dvector.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/tuple/b3dtuple.hxx>
#include <basegfx/range/basicrange.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    class B3DHomMatrix;

    class B3DRange
    {
        typedef ::basegfx::BasicRange< double, DoubleTraits >   MyBasicRange;

        MyBasicRange            maRangeX;
        MyBasicRange            maRangeY;
        MyBasicRange            maRangeZ;

    public:
        B3DRange() {}

        explicit B3DRange(const B3DTuple& rTuple)
        :   maRangeX(rTuple.getX()),
            maRangeY(rTuple.getY()),
            maRangeZ(rTuple.getZ())
        {
        }

        B3DRange(double x1,
                 double y1,
                 double z1,
                 double x2,
                 double y2,
                 double z2)
        :   maRangeX(x1),
            maRangeY(y1),
            maRangeZ(z1)
        {
            maRangeX.expand(x2);
            maRangeY.expand(y2);
            maRangeZ.expand(z2);
        }

        B3DRange(const B3DTuple& rTuple1,
                 const B3DTuple& rTuple2)
        :   maRangeX(rTuple1.getX()),
            maRangeY(rTuple1.getY()),
            maRangeZ(rTuple1.getZ())
        {
            expand(rTuple2);
        }

        bool isEmpty() const
        {
            return (
                maRangeX.isEmpty()
                || maRangeY.isEmpty()
                || maRangeZ.isEmpty()
                );
        }

        void reset()
        {
            maRangeX.reset();
            maRangeY.reset();
            maRangeZ.reset();
        }

        bool operator==( const B3DRange& rRange ) const
        {
            return (maRangeX == rRange.maRangeX
                && maRangeY == rRange.maRangeY
                && maRangeZ == rRange.maRangeZ);
        }

        bool operator!=( const B3DRange& rRange ) const
        {
            return (maRangeX != rRange.maRangeX
                || maRangeY != rRange.maRangeY
                || maRangeZ != rRange.maRangeZ);
        }

        bool equal(const B3DRange& rRange) const
        {
            return (maRangeX.equal(rRange.maRangeX)
                    && maRangeY.equal(rRange.maRangeY)
                    && maRangeZ.equal(rRange.maRangeZ));
        }

        double getMinX() const
        {
            return maRangeX.getMinimum();
        }

        double getMinY() const
        {
            return maRangeY.getMinimum();
        }

        double getMinZ() const
        {
            return maRangeZ.getMinimum();
        }

        double getMaxX() const
        {
            return maRangeX.getMaximum();
        }

        double getMaxY() const
        {
            return maRangeY.getMaximum();
        }

        double getMaxZ() const
        {
            return maRangeZ.getMaximum();
        }

        double getWidth() const
        {
            return maRangeX.getRange();
        }

        double getHeight() const
        {
            return maRangeY.getRange();
        }

        double getDepth() const
        {
            return maRangeZ.getRange();
        }

        B3DPoint getMinimum() const
        {
            return B3DPoint(
                maRangeX.getMinimum(),
                maRangeY.getMinimum(),
                maRangeZ.getMinimum()
                );
        }

        B3DPoint getMaximum() const
        {
            return B3DPoint(
                maRangeX.getMaximum(),
                maRangeY.getMaximum(),
                maRangeZ.getMaximum()
                );
        }

        B3DVector getRange() const
        {
            return B3DVector(
                maRangeX.getRange(),
                maRangeY.getRange(),
                maRangeZ.getRange()
                );
        }

        B3DPoint getCenter() const
        {
            return B3DPoint(
                maRangeX.getCenter(),
                maRangeY.getCenter(),
                maRangeZ.getCenter()
                );
        }

        double getCenterX() const
        {
            return maRangeX.getCenter();
        }

        double getCenterY() const
        {
            return maRangeY.getCenter();
        }

        double getCenterZ() const
        {
            return maRangeZ.getCenter();
        }

        bool isInside(const B3DTuple& rTuple) const
        {
            return (
                maRangeX.isInside(rTuple.getX())
                && maRangeY.isInside(rTuple.getY())
                && maRangeZ.isInside(rTuple.getZ())
                );
        }

        bool isInside(const B3DRange& rRange) const
        {
            return (
                maRangeX.isInside(rRange.maRangeX)
                && maRangeY.isInside(rRange.maRangeY)
                && maRangeZ.isInside(rRange.maRangeZ)
                );
        }

        bool overlaps(const B3DRange& rRange) const
        {
            return (
                maRangeX.overlaps(rRange.maRangeX)
                && maRangeY.overlaps(rRange.maRangeY)
                && maRangeZ.overlaps(rRange.maRangeZ)
                );
        }

        void expand(const B3DTuple& rTuple)
        {
            maRangeX.expand(rTuple.getX());
            maRangeY.expand(rTuple.getY());
            maRangeZ.expand(rTuple.getZ());
        }

        void expand(const B3DRange& rRange)
        {
            maRangeX.expand(rRange.maRangeX);
            maRangeY.expand(rRange.maRangeY);
            maRangeZ.expand(rRange.maRangeZ);
        }

        void intersect(const B3DRange& rRange)
        {
            maRangeX.intersect(rRange.maRangeX);
            maRangeY.intersect(rRange.maRangeY);
            maRangeZ.intersect(rRange.maRangeZ);
        }

        void grow(double fValue)
        {
            maRangeX.grow(fValue);
            maRangeY.grow(fValue);
            maRangeZ.grow(fValue);
        }

         BASEGFX_DLLPUBLIC void transform(const B3DHomMatrix& rMatrix);
    };

} // end of namespace basegfx


#endif // INCLUDED_BASEGFX_RANGE_B3DRANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
