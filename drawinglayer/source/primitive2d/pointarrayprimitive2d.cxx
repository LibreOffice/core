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

#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>



using namespace com::sun::star;



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

        bool PointArrayPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const PointArrayPrimitive2D& rCompare = (PointArrayPrimitive2D&)rPrimitive;

                return (getPositions() == rCompare.getPositions()
                    && getRGBColor() == rCompare.getRGBColor());
            }

            return false;
        }

        basegfx::B2DRange PointArrayPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if(maB2DRange.isEmpty())
            {
                basegfx::B2DRange aNewRange;

                
                for(std::vector< basegfx::B2DPoint >::const_iterator aIter(getPositions().begin()), aEnd(getPositions().end()); aIter != aEnd; ++aIter)
                {
                    aNewRange.expand(*aIter);
                }

                
                const_cast< PointArrayPrimitive2D* >(this)->maB2DRange = aNewRange;
            }

            return maB2DRange;
        }

        
        ImplPrimitive2DIDBlock(PointArrayPrimitive2D, PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D)

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
