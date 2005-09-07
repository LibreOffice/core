/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: liangbarsky.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:51:24 $
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
#include "basegfx/numeric/ftools.hxx"
#include "basegfx/range/b2drange.hxx"


namespace basegfx
{
    namespace tools
    {
        namespace
        {
            // see Foley/vanDam, pp. 122 for the Liang-Barsky line
            // clipping algorithm
            inline bool liangBarskyClipT( double  nDenom,
                                          double  nNumerator,
                                          double& io_rTE,
                                          double& io_rTL )
            {
                double t;
                if( nDenom > 0 )
                {
                    t = nNumerator / nDenom;
                    if( t > io_rTL )
                        return false;
                    else if( t > io_rTE )
                        io_rTE = t;
                }
                else if( nDenom < 0 )
                {
                    t = nNumerator / nDenom;
                    if( t < io_rTE )
                        return false;
                    else
                        io_rTL = t;
                }
                else if( nNumerator > 0 )
                {
                    return false;
                }

                return true;
            }
        }

        // see Foley/vanDam, pp. 122 for the Liang-Barsky line
        // clipping algorithm
        bool liangBarskyClip2D( ::basegfx::B2DPoint&        io_rStart,
                                ::basegfx::B2DPoint&        io_rEnd,
                                const ::basegfx::B2DRange&  rClipRect )
        {
            const double nDX( io_rEnd.getX() - io_rStart.getX() );
            const double nDY( io_rEnd.getY() - io_rStart.getY() );

            if( ::basegfx::fTools::equalZero( nDX ) &&
                ::basegfx::fTools::equalZero( nDY ) )
            {
                return rClipRect.isInside( io_rStart );
            }
            else
            {
                double nTE( 0.0 );
                double nTL( 1.0 );
                if( liangBarskyClipT(nDX, rClipRect.getMinX() - io_rStart.getX(),
                                     nTE, nTL ) )                   // inside wrt. left edge
                {
                    if( liangBarskyClipT(-nDX, io_rStart.getX() - rClipRect.getMaxX(),
                                         nTE, nTL ) )               // inside wrt. right edge
                    {
                        if( liangBarskyClipT(nDY, rClipRect.getMinY() - io_rStart.getY(),
                                             nTE, nTL ) )           // inside wrt. bottom edge
                        {
                            if( liangBarskyClipT(-nDY, io_rStart.getY() - rClipRect.getMaxY(),
                                                 nTE, nTL ) )       // inside wrt. top edge
                            {
                                // compute actual intersection points,
                                // if nTL has changed
                                if( nTL < 1.0 )
                                {
                                    io_rEnd.setX( io_rStart.getX() + nTL*nDX );
                                    io_rEnd.setY( io_rStart.getY() + nTL*nDY );
                                }

                                // compute actual intersection points,
                                // if nTE has changed
                                if( nTE > 0.0 )
                                {
                                    io_rStart.setX( io_rStart.getX() + nTE*nDX );
                                    io_rStart.setY( io_rStart.getY() + nTE*nDY );
                                }

                                // line is (at least partially) visible
                                return true;
                            }
                        }
                    }
                }
            }

            return false;
        }
    }
}
