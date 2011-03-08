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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"

#include "basegfx/tools/tools.hxx"
#include "basegfx/range/b2drange.hxx"

#include <algorithm>


namespace basegfx
{
    namespace tools
    {
        namespace
        {
            inline double distance( const double&                   nX,
                                    const double&                   nY,
                                    const ::basegfx::B2DVector&     rNormal,
                                    const double&                   nC )
            {
                return nX*rNormal.getX() + nY*rNormal.getY() - nC;
            }

            void moveLineOutsideRect( ::basegfx::B2DPoint&          io_rStart,
                                      ::basegfx::B2DPoint&          io_rEnd,
                                      const ::basegfx::B2DVector&   rMoveDirection,
                                      const ::basegfx::B2DRange&    rFitTarget      )
            {
                // calc c for normal line form equation n x - c = 0
                const double nC( rMoveDirection.scalar( io_rStart ) );

                // calc maximum orthogonal distance for all four bound
                // rect corners to the line
                const double nMaxDistance( ::std::max(
                                               0.0,
                                               ::std::max(
                                                   distance(rFitTarget.getMinX(),
                                                            rFitTarget.getMinY(),
                                                            rMoveDirection,
                                                            nC),
                                                   ::std::max(
                                                       distance(rFitTarget.getMinX(),
                                                                rFitTarget.getMaxY(),
                                                                rMoveDirection,
                                                                nC),
                                                       ::std::max(
                                                           distance(rFitTarget.getMaxX(),
                                                                    rFitTarget.getMinY(),
                                                                    rMoveDirection,
                                                                    nC),
                                                           distance(rFitTarget.getMaxX(),
                                                                    rFitTarget.getMaxY(),
                                                                    rMoveDirection,
                                                                    nC) ) ) ) ) );

                // now move line points, such that the bound rect
                // points are all either 'on' or on the negative side
                // of the half-plane
                io_rStart += nMaxDistance*rMoveDirection;
                io_rEnd   += nMaxDistance*rMoveDirection;
            }
        }

        void infiniteLineFromParallelogram( ::basegfx::B2DPoint&        io_rLeftTop,
                                            ::basegfx::B2DPoint&        io_rLeftBottom,
                                            ::basegfx::B2DPoint&        io_rRightTop,
                                            ::basegfx::B2DPoint&        io_rRightBottom,
                                            const ::basegfx::B2DRange&  rFitTarget  )
        {
            // For the top and bottom border line of the
            // parallelogram, we determine the distance to all four
            // corner points of the bound rect (tl, tr, bl, br). When
            // using the unit normal form for lines (n x - c = 0), and
            // choosing n to point 'outwards' the parallelogram, then
            // all bound rect corner points having positive distance
            // to the line lie outside the extended gradient rect, and
            // thus, the corresponding border line must be moved the
            // maximum distance outwards.

            // don't use the top and bottom border line direction, and
            // calculate the normal from them. Instead, use the
            // vertical lines (lt - lb or rt - rb), as they more
            // faithfully represent the direction of the
            // to-be-generated infinite line
            ::basegfx::B2DVector aDirectionVertical( io_rLeftTop - io_rLeftBottom );
            aDirectionVertical.normalize();

            const ::basegfx::B2DVector aNormalTop( aDirectionVertical );
            const ::basegfx::B2DVector aNormalBottom( -aDirectionVertical );

            // now extend parallelogram, such that the bound rect
            // point are included
            moveLineOutsideRect( io_rLeftTop, io_rRightTop, aNormalTop, rFitTarget );
            moveLineOutsideRect( io_rLeftBottom, io_rRightBottom, aNormalBottom, rFitTarget );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
