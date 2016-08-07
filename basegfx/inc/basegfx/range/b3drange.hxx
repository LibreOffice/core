/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _BGFX_RANGE_B3DRANGE_HXX
#define _BGFX_RANGE_B3DRANGE_HXX

#include <basegfx/vector/b3dvector.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/tuple/b3dtuple.hxx>
#include <basegfx/range/basicrange.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    // predeclarations
    class B3IRange;
    class B3DHomMatrix;

    class BASEGFX_DLLPUBLIC B3DRange
    {
        typedef ::basegfx::BasicRange< double, DoubleTraits >   MyBasicRange;

        MyBasicRange            maRangeX;
        MyBasicRange            maRangeY;
        MyBasicRange            maRangeZ;

    public:
        B3DRange()
        {
        }

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

        B3DRange(const B3DRange& rRange)
        :   maRangeX(rRange.maRangeX),
            maRangeY(rRange.maRangeY),
            maRangeZ(rRange.maRangeZ)
        {
        }

        explicit B3DRange(const B3IRange& rRange);

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

        B3DRange& operator=(const B3DRange& rRange)
        {
            maRangeX = rRange.maRangeX;
            maRangeY = rRange.maRangeY;
            maRangeZ = rRange.maRangeZ;
            return *this;
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

        void transform(const B3DHomMatrix& rMatrix);
    };

    /** Round double to nearest integer for 3D range

        @return the nearest integer for this range
    */
    BASEGFX_DLLPUBLIC B3IRange fround(const B3DRange& rRange);
} // end of namespace basegfx


#endif /* _BGFX_RANGE_B3DRANGE_HXX */
