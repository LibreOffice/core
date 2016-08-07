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



#ifndef _BGFX_RANGE_B3IBOX_HXX
#define _BGFX_RANGE_B3IBOX_HXX

#include <basegfx/point/b3ipoint.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/tuple/b3ituple.hxx>
#include <basegfx/tuple/b3i64tuple.hxx>
#include <basegfx/range/basicbox.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    class BASEGFX_DLLPUBLIC B3IBox
    {
        BasicBox            maRangeX;
        BasicBox            maRangeY;
        BasicBox            maRangeZ;

    public:
        B3IBox()
        {
        }

        explicit B3IBox(const B3ITuple& rTuple) :
            maRangeX(rTuple.getX()),
            maRangeY(rTuple.getY()),
            maRangeZ(rTuple.getZ())
        {
        }

        B3IBox(sal_Int32 x1,
               sal_Int32 y1,
               sal_Int32 z1,
               sal_Int32 x2,
               sal_Int32 y2,
               sal_Int32 z2) :
            maRangeX(x1),
            maRangeY(y1),
            maRangeZ(z1)
        {
            maRangeX.expand(x2);
            maRangeY.expand(y2);
            maRangeZ.expand(z2);
        }

        B3IBox(const B3ITuple& rTuple1,
               const B3ITuple& rTuple2) :
            maRangeX(rTuple1.getX()),
            maRangeY(rTuple1.getY()),
            maRangeZ(rTuple1.getZ())
        {
            expand(rTuple2);
        }

        B3IBox(const B3IBox& rBox) :
            maRangeX(rBox.maRangeX),
            maRangeY(rBox.maRangeY),
            maRangeZ(rBox.maRangeZ)
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

        bool operator==( const B3IBox& rBox ) const
        {
            return (maRangeX == rBox.maRangeX
                && maRangeY == rBox.maRangeY
                && maRangeZ == rBox.maRangeZ);
        }

        bool operator!=( const B3IBox& rBox ) const
        {
            return (maRangeX != rBox.maRangeX
                || maRangeY != rBox.maRangeY
                || maRangeZ != rBox.maRangeZ);
        }

        void operator=(const B3IBox& rBox)
        {
            maRangeX = rBox.maRangeX;
            maRangeY = rBox.maRangeY;
            maRangeZ = rBox.maRangeZ;
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

        bool isInside(const B3IBox& rBox) const
        {
            return (
                maRangeX.isInside(rBox.maRangeX)
                && maRangeY.isInside(rBox.maRangeY)
                && maRangeZ.isInside(rBox.maRangeZ)
                );
        }

        bool overlaps(const B3IBox& rBox) const
        {
            return (
                maRangeX.overlaps(rBox.maRangeX)
                && maRangeY.overlaps(rBox.maRangeY)
                && maRangeZ.overlaps(rBox.maRangeZ)
                );
        }

        void expand(const B3ITuple& rTuple)
        {
            maRangeX.expand(rTuple.getX());
            maRangeY.expand(rTuple.getY());
            maRangeZ.expand(rTuple.getZ());
        }

        void expand(const B3IBox& rBox)
        {
            maRangeX.expand(rBox.maRangeX);
            maRangeY.expand(rBox.maRangeY);
            maRangeZ.expand(rBox.maRangeZ);
        }

        void intersect(const B3IBox& rBox)
        {
            maRangeX.intersect(rBox.maRangeX);
            maRangeY.intersect(rBox.maRangeY);
            maRangeZ.intersect(rBox.maRangeZ);
        }

        void grow(sal_Int32 nValue)
        {
            maRangeX.grow(nValue);
            maRangeY.grow(nValue);
            maRangeZ.grow(nValue);
        }
    };
} // end of namespace basegfx

#endif /* _BGFX_RANGE_B3IBOX_HXX */
