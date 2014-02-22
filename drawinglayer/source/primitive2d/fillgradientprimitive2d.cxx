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

#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/texture/texture.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        void FillGradientPrimitive2D::generateMatricesAndColors(
            std::vector< drawinglayer::texture::B2DHomMatrixAndBColor >& rEntries,
            basegfx::BColor& rOutmostColor) const
        {
            rEntries.clear();

            
            const basegfx::BColor aStart(getFillGradient().getStartColor());
            const basegfx::BColor aEnd(getFillGradient().getEndColor());
            const sal_uInt32 nMaxSteps(sal_uInt32((aStart.getMaximumDistance(aEnd) * 127.5) + 0.5));
            sal_uInt32 nSteps(getFillGradient().getSteps());

            if(nSteps == 0)
            {
                nSteps = nMaxSteps;
            }

            if(nSteps < 2)
            {
                nSteps = 2;
            }

            if(nSteps > nMaxSteps)
            {
                nSteps = nMaxSteps;
            }

            nSteps = std::max(sal_uInt32(1), nSteps);

            switch(getFillGradient().getStyle())
            {
                case attribute::GRADIENTSTYLE_LINEAR:
                {
                    texture::GeoTexSvxGradientLinear aGradient(getObjectRange(), aStart, aEnd, nSteps, getFillGradient().getBorder(), getFillGradient().getAngle());
                    aGradient.appendTransformationsAndColors(rEntries, rOutmostColor);
                    break;
                }
                case attribute::GRADIENTSTYLE_AXIAL:
                {
                    texture::GeoTexSvxGradientAxial aGradient(getObjectRange(), aStart, aEnd, nSteps, getFillGradient().getBorder(), getFillGradient().getAngle());
                    aGradient.appendTransformationsAndColors(rEntries, rOutmostColor);
                    break;
                }
                case attribute::GRADIENTSTYLE_RADIAL:
                {
                    texture::GeoTexSvxGradientRadial aGradient(getObjectRange(), aStart, aEnd, nSteps, getFillGradient().getBorder(), getFillGradient().getOffsetX(), getFillGradient().getOffsetY());
                    aGradient.appendTransformationsAndColors(rEntries, rOutmostColor);
                    break;
                }
                case attribute::GRADIENTSTYLE_ELLIPTICAL:
                {
                    texture::GeoTexSvxGradientElliptical aGradient(getObjectRange(), aStart, aEnd, nSteps, getFillGradient().getBorder(), getFillGradient().getOffsetX(), getFillGradient().getOffsetY(), getFillGradient().getAngle());
                    aGradient.appendTransformationsAndColors(rEntries, rOutmostColor);
                    break;
                }
                case attribute::GRADIENTSTYLE_SQUARE:
                {
                    texture::GeoTexSvxGradientSquare aGradient(getObjectRange(), aStart, aEnd, nSteps, getFillGradient().getBorder(), getFillGradient().getOffsetX(), getFillGradient().getOffsetY(), getFillGradient().getAngle());
                    aGradient.appendTransformationsAndColors(rEntries, rOutmostColor);
                    break;
                }
                case attribute::GRADIENTSTYLE_RECT:
                {
                    texture::GeoTexSvxGradientRect aGradient(getObjectRange(), aStart, aEnd, nSteps, getFillGradient().getBorder(), getFillGradient().getOffsetX(), getFillGradient().getOffsetY(), getFillGradient().getAngle());
                    aGradient.appendTransformationsAndColors(rEntries, rOutmostColor);
                    break;
                }
            }
        }

        Primitive2DSequence FillGradientPrimitive2D::createOverlappingFill(
            const std::vector< drawinglayer::texture::B2DHomMatrixAndBColor >& rEntries,
            const basegfx::BColor& rOutmostColor,
            const basegfx::B2DPolygon& rUnitPolygon) const
        {
            
            Primitive2DSequence aRetval(rEntries.size() + 1);

            
            aRetval[0] = Primitive2DReference(
                new PolyPolygonColorPrimitive2D(
                    basegfx::B2DPolyPolygon(basegfx::tools::createPolygonFromRect(getObjectRange())),
                    rOutmostColor));

            
            for(sal_uInt32 a(0); a < rEntries.size(); a++)
            {
                
                basegfx::B2DPolygon aNewPoly(rUnitPolygon);

                aNewPoly.transform(rEntries[a].maB2DHomMatrix);

                
                aRetval[a + 1] = Primitive2DReference(
                    new PolyPolygonColorPrimitive2D(
                        basegfx::B2DPolyPolygon(aNewPoly),
                        rEntries[a].maBColor));
            }

            return aRetval;
        }

        Primitive2DSequence FillGradientPrimitive2D::createNonOverlappingFill(
            const std::vector< drawinglayer::texture::B2DHomMatrixAndBColor >& rEntries,
            const basegfx::BColor& rOutmostColor,
            const basegfx::B2DPolygon& rUnitPolygon) const
        {
            
            Primitive2DSequence aRetval(rEntries.size() + 1);

            
            basegfx::B2DRange aOutmostRange(getObjectRange());
            basegfx::B2DPolyPolygon aCombinedPolyPoly;

            if(rEntries.size())
            {
                
                basegfx::B2DPolygon aFirstPoly(rUnitPolygon);

                aFirstPoly.transform(rEntries[0].maB2DHomMatrix);
                aCombinedPolyPoly.append(aFirstPoly);
                aOutmostRange.expand(aFirstPoly.getB2DRange());
            }

            
            aCombinedPolyPoly.insert(0, basegfx::tools::createPolygonFromRect(aOutmostRange));
            aRetval[0] = Primitive2DReference(
                new PolyPolygonColorPrimitive2D(
                    aCombinedPolyPoly,
                    rOutmostColor));

            if(rEntries.size())
            {
                
                aCombinedPolyPoly.remove(0);

                for(sal_uInt32 a(0); a < rEntries.size() - 1; a++)
                {
                    
                    basegfx::B2DPolygon aNextPoly(rUnitPolygon);

                    aNextPoly.transform(rEntries[a + 1].maB2DHomMatrix);
                    aCombinedPolyPoly.append(aNextPoly);

                    
                    aRetval[a + 1] = Primitive2DReference(
                        new PolyPolygonColorPrimitive2D(
                            aCombinedPolyPoly,
                            rEntries[a].maBColor));

                    
                    aCombinedPolyPoly.remove(0);
                }

                
                aRetval[rEntries.size()] = Primitive2DReference(
                    new PolyPolygonColorPrimitive2D(
                        aCombinedPolyPoly,
                        rEntries[rEntries.size() - 1].maBColor));
            }

            return aRetval;
        }

        Primitive2DSequence FillGradientPrimitive2D::createFill(bool bOverlapping) const
        {
            
            basegfx::B2DPolygon aUnitPolygon;

            switch(getFillGradient().getStyle())
            {
                case attribute::GRADIENTSTYLE_RADIAL:
                case attribute::GRADIENTSTYLE_ELLIPTICAL:
                {
                    aUnitPolygon = basegfx::tools::createPolygonFromCircle(basegfx::B2DPoint(0.0, 0.0), 1.0);
                    break;
                }
                default: 
                {
                    aUnitPolygon = basegfx::tools::createPolygonFromRect(basegfx::B2DRange(-1.0, -1.0, 1.0, 1.0));
                    break;
                }
            }

            
            
            std::vector< drawinglayer::texture::B2DHomMatrixAndBColor > aEntries;
            basegfx::BColor aOutmostColor;

            generateMatricesAndColors(aEntries, aOutmostColor);

            if(bOverlapping)
            {
                return createOverlappingFill(aEntries, aOutmostColor, aUnitPolygon);
            }
            else
            {
                return createNonOverlappingFill(aEntries, aOutmostColor, aUnitPolygon);
            }
        }

        Primitive2DSequence FillGradientPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            
            
            
            
            
            

            if(!getFillGradient().isDefault())
            {
                static bool bOverlapping(true); 

                return createFill(bOverlapping);
            }
            else
            {
                return Primitive2DSequence();
            }
        }

        FillGradientPrimitive2D::FillGradientPrimitive2D(
            const basegfx::B2DRange& rObjectRange,
            const attribute::FillGradientAttribute& rFillGradient)
        :   BufferedDecompositionPrimitive2D(),
            maObjectRange(rObjectRange),
            maFillGradient(rFillGradient)
        {
        }

        bool FillGradientPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const FillGradientPrimitive2D& rCompare = (FillGradientPrimitive2D&)rPrimitive;

                return (getObjectRange() == rCompare.getObjectRange()
                    && getFillGradient() == rCompare.getFillGradient());
            }

            return false;
        }

        basegfx::B2DRange FillGradientPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            
            return getObjectRange();
        }

        
        ImplPrimitive2DIDBlock(FillGradientPrimitive2D, PRIMITIVE2D_ID_FILLGRADIENTPRIMITIVE2D)

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
