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
#include "precompiled_basegfx.hxx"

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
