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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
