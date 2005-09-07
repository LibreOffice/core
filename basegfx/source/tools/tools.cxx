/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tools.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:51:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
