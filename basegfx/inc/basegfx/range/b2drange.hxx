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



#ifndef _BGFX_RANGE_B2DRANGE_HXX
#define _BGFX_RANGE_B2DRANGE_HXX

#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/range/basicrange.hxx>
#include <vector>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    // predeclarations
    class B2IRange;
    class B2DHomMatrix;

    class B2DRange
    {
    public:
        typedef double          ValueType;
        typedef DoubleTraits    TraitsType;

        B2DRange()
        {
        }

        explicit B2DRange(const B2DTuple& rTuple)
        :   maRangeX(rTuple.getX()),
            maRangeY(rTuple.getY())
        {
        }

        B2DRange(double x1,
                 double y1,
                 double x2,
                 double y2)
        :   maRangeX(x1),
            maRangeY(y1)
        {
            maRangeX.expand(x2);
            maRangeY.expand(y2);
        }

        B2DRange(const B2DTuple& rTuple1,
                 const B2DTuple& rTuple2)
        :   maRangeX(rTuple1.getX()),
            maRangeY(rTuple1.getY())
        {
            expand( rTuple2 );
        }

        B2DRange(const B2DRange& rRange)
        :   maRangeX(rRange.maRangeX),
            maRangeY(rRange.maRangeY)
        {
        }

        BASEGFX_DLLPUBLIC explicit B2DRange(const B2IRange& rRange);

        bool isEmpty() const
        {
            return (
                maRangeX.isEmpty()
                || maRangeY.isEmpty()
                );
        }

        void reset()
        {
            maRangeX.reset();
            maRangeY.reset();
        }

        bool operator==( const B2DRange& rRange ) const
        {
            return (maRangeX == rRange.maRangeX
                && maRangeY == rRange.maRangeY);
        }

        bool operator!=( const B2DRange& rRange ) const
        {
            return (maRangeX != rRange.maRangeX
                || maRangeY != rRange.maRangeY);
        }

        B2DRange& operator=(const B2DRange& rRange)
        {
            maRangeX = rRange.maRangeX;
            maRangeY = rRange.maRangeY;
            return *this;
        }

        bool equal(const B2DRange& rRange) const
        {
            return (maRangeX.equal(rRange.maRangeX)
                    && maRangeY.equal(rRange.maRangeY));
        }

        double getMinX() const
        {
            return maRangeX.getMinimum();
        }

        double getMinY() const
        {
            return maRangeY.getMinimum();
        }

        double getMaxX() const
        {
            return maRangeX.getMaximum();
        }

        double getMaxY() const
        {
            return maRangeY.getMaximum();
        }

        double getWidth() const
        {
            return maRangeX.getRange();
        }

        double getHeight() const
        {
            return maRangeY.getRange();
        }

        B2DPoint getMinimum() const
        {
            return B2DPoint(
                maRangeX.getMinimum(),
                maRangeY.getMinimum()
                );
        }

        B2DPoint getMaximum() const
        {
            return B2DPoint(
                maRangeX.getMaximum(),
                maRangeY.getMaximum()
                );
        }

        B2DVector getRange() const
        {
            return B2DVector(
                maRangeX.getRange(),
                maRangeY.getRange()
                );
        }

        B2DPoint getCenter() const
        {
            return B2DPoint(
                maRangeX.getCenter(),
                maRangeY.getCenter()
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

        bool isInside(const B2DTuple& rTuple) const
        {
            return (
                maRangeX.isInside(rTuple.getX())
                && maRangeY.isInside(rTuple.getY())
                );
        }

        bool isInside(const B2DRange& rRange) const
        {
            return (
                maRangeX.isInside(rRange.maRangeX)
                && maRangeY.isInside(rRange.maRangeY)
                );
        }

        bool overlaps(const B2DRange& rRange) const
        {
            return (
                maRangeX.overlaps(rRange.maRangeX)
                && maRangeY.overlaps(rRange.maRangeY)
                );
        }

        bool overlapsMore(const B2DRange& rRange) const
        {
            return (
                maRangeX.overlapsMore(rRange.maRangeX)
                && maRangeY.overlapsMore(rRange.maRangeY)
                );
        }

        void expand(const B2DTuple& rTuple)
        {
            maRangeX.expand(rTuple.getX());
            maRangeY.expand(rTuple.getY());
        }

        void expand(const B2DRange& rRange)
        {
            maRangeX.expand(rRange.maRangeX);
            maRangeY.expand(rRange.maRangeY);
        }

        void intersect(const B2DRange& rRange)
        {
            maRangeX.intersect(rRange.maRangeX);
            maRangeY.intersect(rRange.maRangeY);
        }

        void grow(double fValue)
        {
            maRangeX.grow(fValue);
            maRangeY.grow(fValue);
        }

        BASEGFX_DLLPUBLIC void transform(const B2DHomMatrix& rMatrix);

    private:
        typedef ::basegfx::BasicRange< ValueType, TraitsType >  MyBasicRange;

        MyBasicRange        maRangeX;
        MyBasicRange        maRangeY;
    };

    /** Round double to nearest integer for 2D range

        @return the nearest integer for this range
    */
    BASEGFX_DLLPUBLIC B2IRange fround(const B2DRange& rRange);

    /** Compute the set difference of the two given ranges

        This method calculates the symmetric difference (aka XOR)
        between the two given ranges, and returning the resulting
        ranges. Thus, the result will contain all areas where one, but
        not both ranges lie.

        @param o_rResult
        Result vector. The up to four difference ranges are returned
        within this vector

        @param rFirst
        The first range

        @param rSecond
        The second range

        @return the input vector
     */
    BASEGFX_DLLPUBLIC ::std::vector< B2DRange >& computeSetDifference( ::std::vector< B2DRange >&   o_rResult,
                                                     const B2DRange&            rFirst,
                                                     const B2DRange&            rSecond );

} // end of namespace basegfx


#endif /* _BGFX_RANGE_B2DRANGE_HXX */
