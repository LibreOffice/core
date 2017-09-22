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

#ifndef INCLUDED_BASEGFX_UTILS_TOOLS_HXX
#define INCLUDED_BASEGFX_UTILS_TOOLS_HXX

#include <sal/types.h>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    class B2DPoint;
    class B2DRange;
    class B2DPolyPolygon;

    namespace utils
    {
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

        /** Creates polypolygon with the given number as seven-segment
            digits

            @param fVal
            Value to convert

            @param nTotalDigits
            Total number of digits to display. If less is needed for
            given number, fill space with blanks.

            @param nDecPlaces
            Decimal places to show. When 0, display as integer. When
            negative, fill given number of before-the-decimal point
            with zero.

            @param bLitSegments
            When true, return a polygon containing the segments that
            are 'lit' for the given number. Return un-lit segments
            otherwise.
         */
        BASEGFX_DLLPUBLIC B2DPolyPolygon number2PolyPolygon(double    fVal,
                                                            sal_Int32 nTotalDigits,
                                                            sal_Int32 nDecPlaces,
                                                            bool      bLitSegments=true);
    }
}

#endif // INCLUDED_BASEGFX_UTILS_TOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
