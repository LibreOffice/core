/*************************************************************************
 *
 *  $RCSfile: liangbarsky.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-13 18:00:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
