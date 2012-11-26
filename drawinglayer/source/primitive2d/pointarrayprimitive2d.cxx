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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        PointArrayPrimitive2D::PointArrayPrimitive2D(
            const std::vector< basegfx::B2DPoint >& rPositions,
            const basegfx::BColor& rRGBColor)
        :   BasePrimitive2D(),
            maPositions(rPositions),
            maRGBColor(rRGBColor),
            maB2DRange()
        {
        }

        basegfx::B2DRange PointArrayPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if(maB2DRange.isEmpty())
            {
                basegfx::B2DRange aNewRange;

                // get the basic range from the position vector
                for(std::vector< basegfx::B2DPoint >::const_iterator aIter(getPositions().begin()); aIter != getPositions().end(); aIter++)
                {
                    aNewRange.expand(*aIter);
                }

                // assign to buffered value
                const_cast< PointArrayPrimitive2D* >(this)->maB2DRange = aNewRange;
            }

            return maB2DRange;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PointArrayPrimitive2D, PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
