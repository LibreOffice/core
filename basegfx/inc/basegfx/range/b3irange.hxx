/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BGFX_RANGE_B3IRANGE_HXX
#define _BGFX_RANGE_B3IRANGE_HXX

#include <basegfx/point/b3ipoint.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/tuple/b3ituple.hxx>
#include <basegfx/tuple/b3i64tuple.hxx>
#include <basegfx/range/basicrange.hxx>

namespace basegfx
{
    class B3IRange
    {
        typedef ::basegfx::BasicRange< sal_Int32, Int32Traits > MyBasicRange;

        MyBasicRange            maRangeX;
        MyBasicRange            maRangeY;
        MyBasicRange            maRangeZ;

    public:
        B3IRange()
        {
        }

        explicit B3IRange(const B3ITuple& rTuple)
        :   maRangeX(rTuple.getX()),
            maRangeY(rTuple.getY()),
            maRangeZ(rTuple.getZ())
        {
        }

        B3IRange(sal_Int32 x1,
                 sal_Int32 y1,
                 sal_Int32 z1,
                 sal_Int32 x2,
                 sal_Int32 y2,
                 sal_Int32 z2)
        :   maRangeX(x1),
            maRangeY(y1),
            maRangeZ(z1)
        {
            maRangeX.expand(x2);
            maRangeY.expand(y2);
            maRangeZ.expand(z2);
        }

        B3IRange(const B3ITuple& rTuple1,
                 const B3ITuple& rTuple2)
        :   maRangeX(rTuple1.getX()),
            maRangeY(rTuple1.getY()),
            maRangeZ(rTuple1.getZ())
        {
            expand(rTuple2);
        }

        B3IRange(const B3IRange& rRange)
        :   maRangeX(rRange.maRangeX),
            maRangeY(rRange.maRangeY),
            maRangeZ(rRange.maRangeZ)
        {
        }

        bool isEmpty() const
        {
            return maRangeX.isEmpty() || maRangeY.isEmpty() || maRangeZ.isEmpty();
        }

        void reset()
        {
            maRangeX.reset();
            maRangeY.reset();
            maRangeZ.reset();
        }

        bool operator==( const B3IRange& rRange ) const
        {
            return (maRangeX == rRange.maRangeX
                && maRangeY == rRange.maRangeY
                && maRangeZ == rRange.maRangeZ);
        }

        bool operator!=( const B3IRange& rRange ) const
        {
            return (maRangeX != rRange.maRangeX
                || maRangeY != rRange.maRangeY
                || maRangeZ != rRange.maRangeZ);
        }

        B3IRange& operator=(const B3IRange& rRange)
        {
            maRangeX = rRange.maRangeX;
            maRangeY = rRange.maRangeY;
            maRangeZ = rRange.maRangeZ;
            return *this;
        }

        sal_Int32 getMinX() const
        {
            return maRangeX.getMinimum();
        }

        sal_Int32 getMinY() const
        {
            return maRangeY.getMinimum();
        }

        sal_Int32 getMinZ() const
        {
            return maRangeZ.getMinimum();
        }

        sal_Int32 getMaxX() const
        {
            return maRangeX.getMaximum();
        }

        sal_Int32 getMaxY() const
        {
            return maRangeY.getMaximum();
        }

        sal_Int32 getMaxZ() const
        {
            return maRangeZ.getMaximum();
        }

        sal_Int64 getWidth() const
        {
            return maRangeX.getRange();
        }

        sal_Int64 getHeight() const
        {
            return maRangeY.getRange();
        }

        sal_Int64 getDepth() const
        {
            return maRangeZ.getRange();
        }

        B3IPoint getMinimum() const
        {
            return B3IPoint(
                maRangeX.getMinimum(),
                maRangeY.getMinimum(),
                maRangeZ.getMinimum()
                );
        }

        B3IPoint getMaximum() const
        {
            return B3IPoint(
                maRangeX.getMaximum(),
                maRangeY.getMaximum(),
                maRangeZ.getMaximum()
                );
        }

        B3I64Tuple getRange() const
        {
            return B3I64Tuple(
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

        bool isInside(const B3ITuple& rTuple) const
        {
            return (
                maRangeX.isInside(rTuple.getX())
                && maRangeY.isInside(rTuple.getY())
                && maRangeZ.isInside(rTuple.getZ())
                );
        }

        bool isInside(const B3IRange& rRange) const
        {
            return (
                maRangeX.isInside(rRange.maRangeX)
                && maRangeY.isInside(rRange.maRangeY)
                && maRangeZ.isInside(rRange.maRangeZ)
                );
        }

        bool overlaps(const B3IRange& rRange) const
        {
            return (
                maRangeX.overlaps(rRange.maRangeX)
                && maRangeY.overlaps(rRange.maRangeY)
                && maRangeZ.overlaps(rRange.maRangeZ)
                );
        }

        void expand(const B3ITuple& rTuple)
        {
            maRangeX.expand(rTuple.getX());
            maRangeY.expand(rTuple.getY());
            maRangeZ.expand(rTuple.getZ());
        }

        void expand(const B3IRange& rRange)
        {
            maRangeX.expand(rRange.maRangeX);
            maRangeY.expand(rRange.maRangeY);
            maRangeZ.expand(rRange.maRangeZ);
        }

        void intersect(const B3IRange& rRange)
        {
            maRangeX.intersect(rRange.maRangeX);
            maRangeY.intersect(rRange.maRangeY);
            maRangeZ.intersect(rRange.maRangeZ);
        }

        void grow(sal_Int32 nValue)
        {
            maRangeX.grow(nValue);
            maRangeY.grow(nValue);
            maRangeZ.grow(nValue);
        }
    };
} // end of namespace basegfx

#endif /* _BGFX_RANGE_B3IRANGE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
