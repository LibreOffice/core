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



#ifndef _BGFX_TOOLS_TOOLS_HXX
#define _BGFX_TOOLS_TOOLS_HXX

#include <sal/types.h>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    class B2DPoint;
    class B2DRange;

    namespace tools
    {
        /** Liang-Barsky 2D line clipping algorithm

            This function clips a line given by two points against the
            given rectangle. The resulting line is returned in the
            given points.

            @param io_rStart
            Start point of the line. On return, contains the clipped
            start point.

            @param io_rEnd
            End point of the line. On return, contains the clipped
            end point.

            @param rClipRect
            The rectangle to clip against

            @return true, when at least part of the line is visible
            after the clip, false otherwise
         */
        BASEGFX_DLLPUBLIC bool liangBarskyClip2D( ::basegfx::B2DPoint&      io_rStart,
                                ::basegfx::B2DPoint&        io_rEnd,
                                const ::basegfx::B2DRange&  rClipRect );

        /** Expand given parallelogram, such that it extends beyond
            bound rect in a given direction.

            This method is useful when e.g. generating one-dimensional
            gradients, such as linear or axial gradients: those
            gradients vary only in one direction, the other has
            constant color. Most of the time, those gradients extends
            infinitely in the direction with the constant color, but
            practically, one always has a limiting bound rect into
            which the gradient is painted. The method at hand now
            extends a given parallelogram (e.g. the transformed
            bounding box of a gradient) virtually into infinity to the
            top and to the bottom (i.e. normal to the line io_rLeftTop
            io_rRightTop), such that the given rectangle is guaranteed
            to be covered in that direction.

            @attention There might be some peculiarities with this
            method, that might limit its usage to the described
            gradients. One of them is the fact that when determining
            how far the parallelogram has to be extended to the top or
            the bottom, the upper and lower border are assumed to be
            infinite lines.

            @param io_rLeftTop
            Left, top edge of the parallelogramm. Note that this need
            not be the left, top edge geometrically, it's just used
            when determining the extension direction. Thus, it's
            perfectly legal to affine-transform a rectangle, and given
            the transformed point here. On method return, this
            parameter will contain the adapted output.

            @param io_rLeftBottom
            Left, bottom edge of the parallelogramm. Note that this need
            not be the left, bottom edge geometrically, it's just used
            when determining the extension direction. Thus, it's
            perfectly legal to affine-transform a rectangle, and given
            the transformed point here. On method return, this
            parameter will contain the adapted output.

            @param io_rRightTop
            Right, top edge of the parallelogramm. Note that this need
            not be the right, top edge geometrically, it's just used
            when determining the extension direction. Thus, it's
            perfectly legal to affine-transform a rectangle, and given
            the transformed point here. On method return, this
            parameter will contain the adapted output.

            @param io_rRightBottom
            Right, bottom edge of the parallelogramm. Note that this need
            not be the right, bottom edge geometrically, it's just used
            when determining the extension direction. Thus, it's
            perfectly legal to affine-transform a rectangle, and given
            the transformed point here. On method return, this
            parameter will contain the adapted output.

            @param rFitTarget
            The rectangle to fit the parallelogram into.
         */
        BASEGFX_DLLPUBLIC void infiniteLineFromParallelogram( ::basegfx::B2DPoint&      io_rLeftTop,
                                            ::basegfx::B2DPoint&        io_rLeftBottom,
                                            ::basegfx::B2DPoint&        io_rRightTop,
                                            ::basegfx::B2DPoint&        io_rRightBottom,
                                            const ::basegfx::B2DRange&  rFitTarget  );

    }
}

#endif /* _BGFX_TOOLS_TOOLS_HXX */
