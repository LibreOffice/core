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

#include <drawinglayer/primitive2d/gridprimitive2d.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>


using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
        void GridPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const
        {
            if(!(!rViewInformation.getViewport().isEmpty() && getWidth() > 0.0 && getHeight() > 0.0))
                return;

            // decompose grid matrix to get logic size
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            getTransform().decompose(aScale, aTranslate, fRotate, fShearX);

            // create grid matrix which transforms from scaled logic to view
            basegfx::B2DHomMatrix aRST(basegfx::utils::createShearXRotateTranslateB2DHomMatrix(
                fShearX, fRotate, aTranslate.getX(), aTranslate.getY()));
            aRST *= rViewInformation.getObjectToViewTransformation();

            // get step widths
            double fStepX(getWidth());
            double fStepY(getHeight());
            const double fMinimalStep(10.0);

            // guarantee a step width of 10.0
            if(basegfx::fTools::less(fStepX, fMinimalStep))
            {
                fStepX = fMinimalStep;
            }

            if(basegfx::fTools::less(fStepY, fMinimalStep))
            {
                fStepY = fMinimalStep;
            }

            // get relative distances in view coordinates
            double fViewStepX((rViewInformation.getObjectToViewTransformation() * basegfx::B2DVector(fStepX, 0.0)).getLength());
            double fViewStepY((rViewInformation.getObjectToViewTransformation() * basegfx::B2DVector(0.0, fStepY)).getLength());
            double fSmallStepX(1.0), fViewSmallStepX(1.0), fSmallStepY(1.0), fViewSmallStepY(1.0);
            sal_uInt32 nSmallStepsX(0), nSmallStepsY(0);

            // setup subdivisions
            if(getSubdivisionsX())
            {
                fSmallStepX = fStepX / getSubdivisionsX();
                fViewSmallStepX = fViewStepX / getSubdivisionsX();
            }

            if(getSubdivisionsY())
            {
                fSmallStepY = fStepY / getSubdivisionsY();
                fViewSmallStepY = fViewStepY / getSubdivisionsY();
            }

            // correct step width
            while(fViewStepX < getSmallestViewDistance())
            {
                fViewStepX *= 2.0;
                fStepX *= 2.0;
            }

            while(fViewStepY < getSmallestViewDistance())
            {
                fViewStepY *= 2.0;
                fStepY *= 2.0;
            }

            // correct small step width
            if(getSubdivisionsX())
            {
                while(fViewSmallStepX < getSmallestSubdivisionViewDistance())
                {
                    fViewSmallStepX *= 2.0;
                    fSmallStepX *= 2.0;
                }

                nSmallStepsX = static_cast<sal_uInt32>(fStepX / fSmallStepX);
            }

            if(getSubdivisionsY())
            {
                while(fViewSmallStepY < getSmallestSubdivisionViewDistance())
                {
                    fViewSmallStepY *= 2.0;
                    fSmallStepY *= 2.0;
                }

                nSmallStepsY = static_cast<sal_uInt32>(fStepY / fSmallStepY);
            }

            // calculate extended viewport in which grid points may lie at all
            basegfx::B2DRange aExtendedViewport;

            if(rViewInformation.getDiscreteViewport().isEmpty())
            {
                // not set, use logic size to travel over all potential grid points
                aExtendedViewport = basegfx::B2DRange(0.0, 0.0, aScale.getX(), aScale.getY());
            }
            else
            {
                // transform unit range to discrete view
                aExtendedViewport = basegfx::B2DRange(0.0, 0.0, 1.0, 1.0);
                basegfx::B2DHomMatrix aTrans(rViewInformation.getObjectToViewTransformation() * getTransform());
                aExtendedViewport.transform(aTrans);

                // intersect with visible part
                aExtendedViewport.intersect(rViewInformation.getDiscreteViewport());

                if(!aExtendedViewport.isEmpty())
                {
                    // convert back and apply scale
                    aTrans.invert();
                    aTrans.scale(aScale.getX(), aScale.getY());
                    aExtendedViewport.transform(aTrans);

                    // crop start/end in X/Y to multiples of logical step width
                    const double fHalfCrossSize((rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(3.0, 0.0)).getLength());
                    const double fMinX(floor((aExtendedViewport.getMinX() - fHalfCrossSize) / fStepX) * fStepX);
                    const double fMaxX(ceil((aExtendedViewport.getMaxX() + fHalfCrossSize) / fStepX) * fStepX);
                    const double fMinY(floor((aExtendedViewport.getMinY() - fHalfCrossSize) / fStepY) * fStepY);
                    const double fMaxY(ceil((aExtendedViewport.getMaxY() + fHalfCrossSize) / fStepY) * fStepY);

                    // put to aExtendedViewport and crop on object logic size
                    aExtendedViewport = basegfx::B2DRange(
                        std::max(fMinX, 0.0),
                        std::max(fMinY, 0.0),
                        std::min(fMaxX, aScale.getX()),
                        std::min(fMaxY, aScale.getY()));
                }
            }

            if(aExtendedViewport.isEmpty())
                return;

            // prepare point vectors for point and cross markers
            std::vector< basegfx::B2DPoint > aPositionsPoint;
            std::vector< basegfx::B2DPoint > aPositionsCross;

            for(double fX(aExtendedViewport.getMinX()); fX < aExtendedViewport.getMaxX(); fX += fStepX)
            {
                const bool bXZero(basegfx::fTools::equalZero(fX));

                for(double fY(aExtendedViewport.getMinY()); fY < aExtendedViewport.getMaxY(); fY += fStepY)
                {
                    const bool bYZero(basegfx::fTools::equalZero(fY));

                    if(!bXZero && !bYZero)
                    {
                        // get discrete position and test against 3x3 area surrounding it
                        // since it's a cross
                        const double fHalfCrossSize(3.0 * 0.5);
                        const basegfx::B2DPoint aViewPos(aRST * basegfx::B2DPoint(fX, fY));
                        const basegfx::B2DRange aDiscreteRangeCross(
                            aViewPos.getX() - fHalfCrossSize, aViewPos.getY() - fHalfCrossSize,
                            aViewPos.getX() + fHalfCrossSize, aViewPos.getY() + fHalfCrossSize);

                        if(rViewInformation.getDiscreteViewport().overlaps(aDiscreteRangeCross))
                        {
                            const basegfx::B2DPoint aLogicPos(rViewInformation.getInverseObjectToViewTransformation() * aViewPos);
                            aPositionsCross.push_back(aLogicPos);
                        }
                    }

                    if(getSubdivisionsX() && !bYZero)
                    {
                        double fF(fX + fSmallStepX);

                        for(sal_uInt32 a(1); a < nSmallStepsX && fF < aExtendedViewport.getMaxX(); a++, fF += fSmallStepX)
                        {
                            const basegfx::B2DPoint aViewPos(aRST * basegfx::B2DPoint(fF, fY));

                            if(rViewInformation.getDiscreteViewport().isInside(aViewPos))
                            {
                                const basegfx::B2DPoint aLogicPos(rViewInformation.getInverseObjectToViewTransformation() * aViewPos);
                                aPositionsPoint.push_back(aLogicPos);
                            }
                        }
                    }

                    if(getSubdivisionsY() && !bXZero)
                    {
                        double fF(fY + fSmallStepY);

                        for(sal_uInt32 a(1); a < nSmallStepsY && fF < aExtendedViewport.getMaxY(); a++, fF += fSmallStepY)
                        {
                            const basegfx::B2DPoint aViewPos(aRST * basegfx::B2DPoint(fX, fF));

                            if(rViewInformation.getDiscreteViewport().isInside(aViewPos))
                            {
                                const basegfx::B2DPoint aLogicPos(rViewInformation.getInverseObjectToViewTransformation() * aViewPos);
                                aPositionsPoint.push_back(aLogicPos);
                            }
                        }
                    }
                }
            }

            // prepare return value
            const sal_uInt32 nCountPoint(aPositionsPoint.size());
            const sal_uInt32 nCountCross(aPositionsCross.size());

            // add PointArrayPrimitive2D if point markers were added
            if(nCountPoint)
            {
                rContainer.push_back(new PointArrayPrimitive2D(std::move(aPositionsPoint), getBColor()));
            }

            // add MarkerArrayPrimitive2D if cross markers were added
            if(!nCountCross)
                return;

            if(!getSubdivisionsX() && !getSubdivisionsY())
            {
                // no subdivisions, so fall back to points at grid positions, no need to
                // visualize a difference between divisions and sub-divisions
                rContainer.push_back(new PointArrayPrimitive2D(std::move(aPositionsCross), getBColor()));
            }
            else
            {
                rContainer.push_back(new MarkerArrayPrimitive2D(std::move(aPositionsCross), getCrossMarker()));
            }
        }

        GridPrimitive2D::GridPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            double fWidth,
            double fHeight,
            double fSmallestViewDistance,
            double fSmallestSubdivisionViewDistance,
            sal_uInt32 nSubdivisionsX,
            sal_uInt32 nSubdivisionsY,
            const basegfx::BColor& rBColor,
            const BitmapEx& rCrossMarker)
        :   maTransform(rTransform),
            mfWidth(fWidth),
            mfHeight(fHeight),
            mfSmallestViewDistance(fSmallestViewDistance),
            mfSmallestSubdivisionViewDistance(fSmallestSubdivisionViewDistance),
            mnSubdivisionsX(nSubdivisionsX),
            mnSubdivisionsY(nSubdivisionsY),
            maBColor(rBColor),
            maCrossMarker(rCrossMarker)
        {
        }

        bool GridPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const GridPrimitive2D& rCompare = static_cast<const GridPrimitive2D&>(rPrimitive);

                return (getTransform() == rCompare.getTransform()
                    && getWidth() == rCompare.getWidth()
                    && getHeight() == rCompare.getHeight()
                    && getSmallestViewDistance() == rCompare.getSmallestViewDistance()
                    && getSmallestSubdivisionViewDistance() == rCompare.getSmallestSubdivisionViewDistance()
                    && getSubdivisionsX() == rCompare.getSubdivisionsX()
                    && getSubdivisionsY() == rCompare.getSubdivisionsY()
                    && getBColor() == rCompare.getBColor()
                    && getCrossMarker() == rCompare.getCrossMarker());
            }

            return false;
        }

        basegfx::B2DRange GridPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            // get object's range
            basegfx::B2DRange aUnitRange(0.0, 0.0, 1.0, 1.0);
            aUnitRange.transform(getTransform());

            // intersect with visible part
            aUnitRange.intersect(rViewInformation.getViewport());

            return aUnitRange;
        }

        void GridPrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const
        {
            if(!getBuffered2DDecomposition().empty())
            {
                if(maLastViewport != rViewInformation.getViewport() || maLastObjectToViewTransformation != rViewInformation.getObjectToViewTransformation())
                {
                    // conditions of last local decomposition have changed, delete
                    const_cast< GridPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DContainer());
                }
            }

            if(getBuffered2DDecomposition().empty())
            {
                // remember ViewRange and ViewTransformation
                const_cast< GridPrimitive2D* >(this)->maLastObjectToViewTransformation = rViewInformation.getObjectToViewTransformation();
                const_cast< GridPrimitive2D* >(this)->maLastViewport = rViewInformation.getViewport();
            }

            // use parent implementation
            BufferedDecompositionPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
        }

        // provide unique ID
        sal_uInt32 GridPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_GRIDPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
