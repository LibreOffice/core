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

#include <drawinglayer/primitive2d/helplineprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/PolygonMarkerPrimitive2D.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>


using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
        Primitive2DReference HelplinePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            if(rViewInformation.getViewport().isEmpty() || getDirection().equalZero())
                return nullptr;

            // position to view coordinates, DashLen and DashLen in logic
            const basegfx::B2DPoint aViewPosition(rViewInformation.getObjectToViewTransformation() * getPosition());
            Primitive2DReference xRet;
            switch(getStyle())
            {
                default : // HelplineStyle2D::Point
                {
                    const double fViewFixValue(15.0);
                    basegfx::B2DVector aNormalizedDirection(getDirection());
                    aNormalizedDirection.normalize();
                    aNormalizedDirection *= fViewFixValue;
                    const basegfx::B2DPoint aStartA(aViewPosition - aNormalizedDirection);
                    const basegfx::B2DPoint aEndA(aViewPosition + aNormalizedDirection);
                    basegfx::B2DPolygon aLineA;
                    aLineA.append(aStartA);
                    aLineA.append(aEndA);
                    aLineA.transform(rViewInformation.getInverseObjectToViewTransformation());
                    auto xMarker1 = new PolygonMarkerPrimitive2D(std::move(aLineA), getRGBColA(), getRGBColB(), getDiscreteDashLength());

                    const basegfx::B2DVector aPerpendicularNormalizedDirection(basegfx::getPerpendicular(aNormalizedDirection));
                    const basegfx::B2DPoint aStartB(aViewPosition - aPerpendicularNormalizedDirection);
                    const basegfx::B2DPoint aEndB(aViewPosition + aPerpendicularNormalizedDirection);
                    basegfx::B2DPolygon aLineB;
                    aLineB.append(aStartB);
                    aLineB.append(aEndB);
                    aLineB.transform(rViewInformation.getInverseObjectToViewTransformation());
                    auto xMarker2 = new PolygonMarkerPrimitive2D(std::move(aLineB), getRGBColA(), getRGBColB(), getDiscreteDashLength());

                    xRet = new GroupPrimitive2D(Primitive2DContainer{xMarker1, xMarker2});
                    break;
                }
                case HelplineStyle2D::Line :
                {
                    basegfx::B2DPolygon aLine;

                    if(basegfx::areParallel(getDirection(), basegfx::B2DVector(1.0, 0.0)))
                    {
                        // parallel to X-Axis, get cuts with Y-Axes
                        const double fCutA((rViewInformation.getDiscreteViewport().getMinX() - aViewPosition.getX()) / getDirection().getX());
                        const double fCutB((rViewInformation.getDiscreteViewport().getMaxX() - aViewPosition.getX()) / getDirection().getX());
                        const basegfx::B2DPoint aPosA(aViewPosition + (fCutA * getDirection()));
                        const basegfx::B2DPoint aPosB(aViewPosition + (fCutB * getDirection()));
                        const bool bBothLeft(aPosA.getX() < rViewInformation.getDiscreteViewport().getMinX() && aPosB.getX() < rViewInformation.getDiscreteViewport().getMinX());
                        const bool bBothRight(aPosA.getX() > rViewInformation.getDiscreteViewport().getMaxX() && aPosB.getX() < rViewInformation.getDiscreteViewport().getMaxX());

                        if(!bBothLeft && !bBothRight)
                        {
                            aLine.append(aPosA);
                            aLine.append(aPosB);
                        }
                    }
                    else
                    {
                        // get cuts with X-Axes
                        const double fCutA((rViewInformation.getDiscreteViewport().getMinY() - aViewPosition.getY()) / getDirection().getY());
                        const double fCutB((rViewInformation.getDiscreteViewport().getMaxY() - aViewPosition.getY()) / getDirection().getY());
                        const basegfx::B2DPoint aPosA(aViewPosition + (fCutA * getDirection()));
                        const basegfx::B2DPoint aPosB(aViewPosition + (fCutB * getDirection()));
                        const bool bBothAbove(aPosA.getY() < rViewInformation.getDiscreteViewport().getMinY() && aPosB.getY() < rViewInformation.getDiscreteViewport().getMinY());
                        const bool bBothBelow(aPosA.getY() > rViewInformation.getDiscreteViewport().getMaxY() && aPosB.getY() < rViewInformation.getDiscreteViewport().getMaxY());

                        if(!bBothAbove && !bBothBelow)
                        {
                            aLine.append(aPosA);
                            aLine.append(aPosB);
                        }
                    }

                    if(aLine.count())
                    {
                        // clip against visible area
                        const basegfx::B2DPolyPolygon aResult(basegfx::utils::clipPolygonOnRange(aLine, rViewInformation.getDiscreteViewport(), true, true));
                        Primitive2DContainer aContainer;
                        for(sal_uInt32 a(0); a < aResult.count(); a++)
                        {
                            basegfx::B2DPolygon aPart(aResult.getB2DPolygon(a));
                            aPart.transform(rViewInformation.getInverseObjectToViewTransformation());
                            aContainer.push_back(new PolygonMarkerPrimitive2D(std::move(aPart), getRGBColA(), getRGBColB(), getDiscreteDashLength()));
                        }
                        xRet = new GroupPrimitive2D(std::move(aContainer));
                    }

                    break;
                }
            }
            return xRet;
        }

        HelplinePrimitive2D::HelplinePrimitive2D(
            const basegfx::B2DPoint& rPosition,
            const basegfx::B2DVector& rDirection,
            HelplineStyle2D eStyle,
            const basegfx::BColor& rRGBColA,
            const basegfx::BColor& rRGBColB,
            double fDiscreteDashLength)
        :   maPosition(rPosition),
            maDirection(rDirection),
            meStyle(eStyle),
            maRGBColA(rRGBColA),
            maRGBColB(rRGBColB),
            mfDiscreteDashLength(fDiscreteDashLength)
        {
        }

        bool HelplinePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const HelplinePrimitive2D& rCompare = static_cast<const HelplinePrimitive2D&>(rPrimitive);

                return (getPosition() == rCompare.getPosition()
                    && getDirection() == rCompare.getDirection()
                    && getStyle() == rCompare.getStyle()
                    && getRGBColA() == rCompare.getRGBColA()
                    && getRGBColB() == rCompare.getRGBColB()
                    && getDiscreteDashLength() == rCompare.getDiscreteDashLength());
            }

            return false;
        }

        void HelplinePrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const
        {
            if(hasBuffered2DDecomposition())
            {
                if(maLastViewport != rViewInformation.getViewport() || maLastObjectToViewTransformation != rViewInformation.getObjectToViewTransformation())
                {
                    // conditions of last local decomposition have changed, delete
                    const_cast< HelplinePrimitive2D* >(this)->setBuffered2DDecomposition(nullptr);
                }
            }

            if(!hasBuffered2DDecomposition())
            {
                // remember ViewRange and ViewTransformation
                const_cast< HelplinePrimitive2D* >(this)->maLastObjectToViewTransformation = rViewInformation.getObjectToViewTransformation();
                const_cast< HelplinePrimitive2D* >(this)->maLastViewport = rViewInformation.getViewport();
            }

            // use parent implementation
            BufferedDecompositionPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
        }

        // provide unique ID
        sal_uInt32 HelplinePrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_HELPLINEPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
