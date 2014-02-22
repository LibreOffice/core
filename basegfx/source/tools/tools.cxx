/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <basegfx/tools/tools.hxx>
#include <basegfx/range/b2drange.hxx>

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
                
                const double nC( rMoveDirection.scalar( io_rStart ) );

                
                
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
            
            
            
            
            
            
            
            
            

            
            
            
            
            
            ::basegfx::B2DVector aDirectionVertical( io_rLeftTop - io_rLeftBottom );
            aDirectionVertical.normalize();

            const ::basegfx::B2DVector aNormalTop( aDirectionVertical );
            const ::basegfx::B2DVector aNormalBottom( -aDirectionVertical );

            
            
            moveLineOutsideRect( io_rLeftTop, io_rRightTop, aNormalTop, rFitTarget );
            moveLineOutsideRect( io_rLeftBottom, io_rRightBottom, aNormalBottom, rFitTarget );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
