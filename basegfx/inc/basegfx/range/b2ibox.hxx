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



#ifndef _BGFX_RANGE_B2IBOX_HXX
#define _BGFX_RANGE_B2IBOX_HXX

#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tuple/b2ituple.hxx>
#include <basegfx/tuple/b2i64tuple.hxx>
#include <basegfx/range/basicbox.hxx>
#include <vector>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    class BASEGFX_DLLPUBLIC B2IBox
    {
    public:
        typedef sal_Int32       ValueType;
        typedef Int32Traits     TraitsType;

        B2IBox()
        {
        }

        explicit B2IBox(const B2ITuple& rTuple)
        :   maRangeX(rTuple.getX()),
            maRangeY(rTuple.getY())
        {
        }

        B2IBox(sal_Int32 x1,
               sal_Int32 y1,
               sal_Int32 x2,
               sal_Int32 y2) :
            maRangeX(x1),
            maRangeY(y1)
        {
            maRangeX.expand(x2);
            maRangeY.expand(y2);
        }

        B2IBox(const B2ITuple& rTuple1,
               const B2ITuple& rTuple2) :
            maRangeX(rTuple1.getX()),
            maRangeY(rTuple1.getY())
        {
            expand( rTuple2 );
        }

        B2IBox(const B2IBox& rBox) :
            maRangeX(rBox.maRangeX),
            maRangeY(rBox.maRangeY)
        {
        }

        bool isEmpty() const
        {
            return maRangeX.isEmpty() || maRangeY.isEmpty();
        }

        void reset()
        {
            maRangeX.reset();
            maRangeY.reset();
        }

        bool operator==( const B2IBox& rBox ) const
        {
            return (maRangeX == rBox.maRangeX
                && maRangeY == rBox.maRangeY);
        }

        bool operator!=( const B2IBox& rBox ) const
        {
            return (maRangeX != rBox.maRangeX
                || maRangeY != rBox.maRangeY);
        }

        void operator=(const B2IBox& rBox)
        {
            maRangeX = rBox.maRangeX;
            maRangeY = rBox.maRangeY;
        }

        sal_Int32 getMinX() const
        {
            return maRangeX.getMinimum();
        }

        sal_Int32 getMinY() const
        {
            return maRangeY.getMinimum();
        }

        sal_Int32 getMaxX() const
        {
            return maRangeX.getMaximum();
        }

        sal_Int32 getMaxY() const
        {
            return maRangeY.getMaximum();
        }

        sal_Int64 getWidth() const
        {
            return maRangeX.getRange();
        }

        sal_Int64 getHeight() const
        {
            return maRangeY.getRange();
        }

        B2IPoint getMinimum() const
        {
            return B2IPoint(
                maRangeX.getMinimum(),
                maRangeY.getMinimum()
                );
        }

        B2IPoint getMaximum() const
        {
            return B2IPoint(
                maRangeX.getMaximum(),
                maRangeY.getMaximum()
                );
        }

        B2I64Tuple getRange() const
        {
            return B2I64Tuple(
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

        bool isInside(const B2ITuple& rTuple) const
        {
            return (
                maRangeX.isInside(rTuple.getX())
                && maRangeY.isInside(rTuple.getY())
                );
        }

        bool isInside(const B2IBox& rBox) const
        {
            return (
                maRangeX.isInside(rBox.maRangeX)
                && maRangeY.isInside(rBox.maRangeY)
                );
        }

        bool overlaps(const B2IBox& rBox) const
        {
            return (
                maRangeX.overlaps(rBox.maRangeX)
                && maRangeY.overlaps(rBox.maRangeY)
                );
        }

        void expand(const B2ITuple& rTuple)
        {
            maRangeX.expand(rTuple.getX());
            maRangeY.expand(rTuple.getY());
        }

        void expand(const B2IBox& rBox)
        {
            maRangeX.expand(rBox.maRangeX);
            maRangeY.expand(rBox.maRangeY);
        }

        void intersect(const B2IBox& rBox)
        {
            maRangeX.intersect(rBox.maRangeX);
            maRangeY.intersect(rBox.maRangeY);
        }

        void grow(sal_Int32 nValue)
        {
            maRangeX.grow(nValue);
            maRangeY.grow(nValue);
        }

    private:
        BasicBox        maRangeX;
        BasicBox        maRangeY;
    };

    /** Compute the set difference of the two given boxes

        This method calculates the symmetric difference (aka XOR)
        between the two given boxes, and returning the resulting
        boxes. Thus, the result will contain all areas where one, but
        not both boxes lie.

        @param o_rResult
        Result vector. The up to four difference boxes are returned
        within this vector

        @param rFirst
        The first box

        @param rSecond
        The second box

        @return the input vector
     */
    ::std::vector< B2IBox >& computeSetDifference( ::std::vector< B2IBox >& o_rResult,
                                                   const B2IBox&            rFirst,
                                                   const B2IBox&            rSecond );

} // end of namespace basegfx

#endif /* _BGFX_RANGE_B2IBOX_HXX */
