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

#include <drawinglayer/primitive2d/sceneprimitive2d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/processor3d/zbufferprocessor3d.hxx>
#include <drawinglayer/processor3d/shadow3dextractor.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <drawinglayer/processor3d/geometry2dextractor.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>



using namespace com::sun::star;



namespace drawinglayer
{
    namespace primitive2d
    {
        bool ScenePrimitive2D::impGetShadow3D(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // create on demand
            if(!mbShadow3DChecked && getChildren3D().hasElements())
            {
                basegfx::B3DVector aLightNormal;
                const double fShadowSlant(getSdrSceneAttribute().getShadowSlant());
                const basegfx::B3DRange aScene3DRange(primitive3d::getB3DRangeFromPrimitive3DSequence(getChildren3D(), getViewInformation3D()));

                if(maSdrLightingAttribute.getLightVector().size())
                {
                    // get light normal from first light and normalize
                    aLightNormal = maSdrLightingAttribute.getLightVector()[0].getDirection();
                    aLightNormal.normalize();
                }

                // create shadow extraction processor
                processor3d::Shadow3DExtractingProcessor aShadowProcessor(
                    getViewInformation3D(),
                    getObjectTransformation(),
                    aLightNormal,
                    fShadowSlant,
                    aScene3DRange);

                // process local primitives
                aShadowProcessor.process(getChildren3D());

                // fetch result and set checked flag
                const_cast< ScenePrimitive2D* >(this)->maShadowPrimitives = aShadowProcessor.getPrimitive2DSequence();
                const_cast< ScenePrimitive2D* >(this)->mbShadow3DChecked = true;
            }

            // return if there are shadow primitives
            return maShadowPrimitives.hasElements();
        }

        void ScenePrimitive2D::calculateDiscreteSizes(
            const geometry::ViewInformation2D& rViewInformation,
            basegfx::B2DRange& rDiscreteRange,
            basegfx::B2DRange& rVisibleDiscreteRange,
            basegfx::B2DRange& rUnitVisibleRange) const
        {
            // use unit range and transform to discrete coordinates
            rDiscreteRange = basegfx::B2DRange(0.0, 0.0, 1.0, 1.0);
            rDiscreteRange.transform(rViewInformation.getObjectToViewTransformation() * getObjectTransformation());

            // clip it against discrete Viewport (if set)
            rVisibleDiscreteRange = rDiscreteRange;

            if(!rViewInformation.getViewport().isEmpty())
            {
                rVisibleDiscreteRange.intersect(rViewInformation.getDiscreteViewport());
            }

            if(rVisibleDiscreteRange.isEmpty())
            {
                rUnitVisibleRange = rVisibleDiscreteRange;
            }
            else
            {
                // create UnitVisibleRange containing unit range values [0.0 .. 1.0] describing
                // the relative position of rVisibleDiscreteRange inside rDiscreteRange
                const double fDiscreteScaleFactorX(basegfx::fTools::equalZero(rDiscreteRange.getWidth()) ? 1.0 : 1.0 / rDiscreteRange.getWidth());
                const double fDiscreteScaleFactorY(basegfx::fTools::equalZero(rDiscreteRange.getHeight()) ? 1.0 : 1.0 / rDiscreteRange.getHeight());

                const double fMinX(basegfx::fTools::equal(rVisibleDiscreteRange.getMinX(), rDiscreteRange.getMinX())
                    ? 0.0
                    : (rVisibleDiscreteRange.getMinX() - rDiscreteRange.getMinX()) * fDiscreteScaleFactorX);
                const double fMinY(basegfx::fTools::equal(rVisibleDiscreteRange.getMinY(), rDiscreteRange.getMinY())
                    ? 0.0
                    : (rVisibleDiscreteRange.getMinY() - rDiscreteRange.getMinY()) * fDiscreteScaleFactorY);

                const double fMaxX(basegfx::fTools::equal(rVisibleDiscreteRange.getMaxX(), rDiscreteRange.getMaxX())
                    ? 1.0
                    : (rVisibleDiscreteRange.getMaxX() - rDiscreteRange.getMinX()) * fDiscreteScaleFactorX);
                const double fMaxY(basegfx::fTools::equal(rVisibleDiscreteRange.getMaxY(), rDiscreteRange.getMaxY())
                    ? 1.0
                    : (rVisibleDiscreteRange.getMaxY() - rDiscreteRange.getMinY()) * fDiscreteScaleFactorY);

                rUnitVisibleRange = basegfx::B2DRange(fMinX, fMinY, fMaxX, fMaxY);
            }
        }

        Primitive2DSequence ScenePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DSequence aRetval;

            // create 2D shadows from contained 3D primitives. This creates the shadow primitives on demand and tells if
            // there are some or not. Do this at start, the shadow might still be visible even when the scene is not
            if(impGetShadow3D(rViewInformation))
            {
                // test visibility
                const basegfx::B2DRange aShadow2DRange(
                    getB2DRangeFromPrimitive2DSequence(maShadowPrimitives, rViewInformation));
                const basegfx::B2DRange aViewRange(
                    rViewInformation.getViewport());

                if(aViewRange.isEmpty() || aShadow2DRange.overlaps(aViewRange))
                {
                    // add extracted 2d shadows (before 3d scene creations itself)
                    aRetval = maShadowPrimitives;
                }
            }

            // get the involved ranges (see helper method calculateDiscreteSizes for details)
            basegfx::B2DRange aDiscreteRange;
            basegfx::B2DRange aVisibleDiscreteRange;
            basegfx::B2DRange aUnitVisibleRange;

            calculateDiscreteSizes(rViewInformation, aDiscreteRange, aVisibleDiscreteRange, aUnitVisibleRange);

            if(!aVisibleDiscreteRange.isEmpty())
            {
                // test if discrete view size (pixel) maybe too big and limit it
                double fViewSizeX(aVisibleDiscreteRange.getWidth());
                double fViewSizeY(aVisibleDiscreteRange.getHeight());
                const double fViewVisibleArea(fViewSizeX * fViewSizeY);
                const SvtOptionsDrawinglayer aDrawinglayerOpt;
                const double fMaximumVisibleArea(aDrawinglayerOpt.GetQuadratic3DRenderLimit());
                double fReduceFactor(1.0);

                if(fViewVisibleArea > fMaximumVisibleArea)
                {
                    fReduceFactor = sqrt(fMaximumVisibleArea / fViewVisibleArea);
                    fViewSizeX *= fReduceFactor;
                    fViewSizeY *= fReduceFactor;
                }

                if(rViewInformation.getReducedDisplayQuality())
                {
                    // when reducing the visualisation is allowed (e.g. an OverlayObject
                    // only needed for dragging), reduce resolution extra
                    // to speed up dragging interactions
                    const double fArea(fViewSizeX * fViewSizeY);
                    double fReducedVisualisationFactor(1.0 / (sqrt(fArea) * (1.0 / 170.0)));

                    if(fReducedVisualisationFactor > 1.0)
                    {
                        fReducedVisualisationFactor = 1.0;
                    }
                    else if(fReducedVisualisationFactor < 0.20)
                    {
                        fReducedVisualisationFactor = 0.20;
                    }

                    if(fReducedVisualisationFactor != 1.0)
                    {
                        fReduceFactor *= fReducedVisualisationFactor;
                    }
                }

                // determine the oversample value
                static sal_uInt16 nDefaultOversampleValue(3);
                const sal_uInt16 nOversampleValue(aDrawinglayerOpt.IsAntiAliasing() ? nDefaultOversampleValue : 0);

                geometry::ViewInformation3D aViewInformation3D(getViewInformation3D());
                {
                    // calculate a transformation from DiscreteRange to evtl. rotated/sheared content.
                    // Start with full transformation from object to discrete units
                    basegfx::B2DHomMatrix aObjToUnit(rViewInformation.getObjectToViewTransformation() * getObjectTransformation());

                    // bring to unit coordinates by applying inverse DiscreteRange
                    aObjToUnit.translate(-aDiscreteRange.getMinX(), -aDiscreteRange.getMinY());
                    aObjToUnit.scale(1.0 / aDiscreteRange.getWidth(), 1.0 / aDiscreteRange.getHeight());

                    // calculate transformed user coordinate system
                    const basegfx::B2DPoint aStandardNull(0.0, 0.0);
                    const basegfx::B2DPoint aUnitRangeTopLeft(aObjToUnit * aStandardNull);
                    const basegfx::B2DVector aStandardXAxis(1.0, 0.0);
                    const basegfx::B2DVector aUnitRangeXAxis(aObjToUnit * aStandardXAxis);
                    const basegfx::B2DVector aStandardYAxis(0.0, 1.0);
                    const basegfx::B2DVector aUnitRangeYAxis(aObjToUnit * aStandardYAxis);

                    if(!aUnitRangeTopLeft.equal(aStandardNull) || !aUnitRangeXAxis.equal(aStandardXAxis) || !aUnitRangeYAxis.equal(aStandardYAxis))
                    {
                        // build transformation from unit range to user coordinate system; the unit range
                        // X and Y axes are the column vectors, the null point is the offset
                        basegfx::B2DHomMatrix aUnitRangeToUser;

                        aUnitRangeToUser.set3x2(
                            aUnitRangeXAxis.getX(), aUnitRangeYAxis.getX(), aUnitRangeTopLeft.getX(),
                            aUnitRangeXAxis.getY(), aUnitRangeYAxis.getY(), aUnitRangeTopLeft.getY());

                        // decompose to allow to apply this to the 3D transformation
                        basegfx::B2DVector aScale, aTranslate;
                        double fRotate, fShearX;
                        aUnitRangeToUser.decompose(aScale, aTranslate, fRotate, fShearX);

                        // apply before DeviceToView and after Projection, 3D is in range [-1.0 .. 1.0] in X,Y and Z
                        // and not yet flipped in Y
                        basegfx::B3DHomMatrix aExtendedProjection(aViewInformation3D.getProjection());

                        // bring to unit coordiantes, flip Y, leave Z unchanged
                        aExtendedProjection.scale(0.5, -0.5, 1.0);
                        aExtendedProjection.translate(0.5, 0.5, 0.0);

                        // apply extra; Y is flipped now, go with positive shear and rotate values
                        aExtendedProjection.scale(aScale.getX(), aScale.getY(), 1.0);
                        aExtendedProjection.shearXZ(fShearX, 0.0);
                        aExtendedProjection.rotate(0.0, 0.0, fRotate);
                        aExtendedProjection.translate(aTranslate.getX(), aTranslate.getY(), 0.0);

                        // back to state after projection
                        aExtendedProjection.translate(-0.5, -0.5, 0.0);
                        aExtendedProjection.scale(2.0, -2.0, 1.0);

                        aViewInformation3D = geometry::ViewInformation3D(
                            aViewInformation3D.getObjectTransformation(),
                            aViewInformation3D.getOrientation(),
                            aExtendedProjection,
                            aViewInformation3D.getDeviceToView(),
                            aViewInformation3D.getViewTime(),
                            aViewInformation3D.getExtendedInformationSequence());
                    }
                }

                // calculate logic render size in world coordinates for usage in renderer
                const basegfx::B2DHomMatrix aInverseOToV(rViewInformation.getInverseObjectToViewTransformation());
                const double fLogicX((aInverseOToV * basegfx::B2DVector(aDiscreteRange.getWidth() * fReduceFactor, 0.0)).getLength());
                const double fLogicY((aInverseOToV * basegfx::B2DVector(0.0, aDiscreteRange.getHeight() * fReduceFactor)).getLength());

                // use default 3D primitive processor to create BitmapEx for aUnitVisiblePart and process
                processor3d::ZBufferProcessor3D aZBufferProcessor3D(
                    aViewInformation3D,
                    rViewInformation,
                    getSdrSceneAttribute(),
                    getSdrLightingAttribute(),
                    fLogicX,
                    fLogicY,
                    aUnitVisibleRange,
                    nOversampleValue);

                aZBufferProcessor3D.process(getChildren3D());
                aZBufferProcessor3D.finish();

                const_cast< ScenePrimitive2D* >(this)->maOldRenderedBitmap = aZBufferProcessor3D.getBitmapEx();
                const Size aBitmapSizePixel(maOldRenderedBitmap.GetSizePixel());

                if(aBitmapSizePixel.getWidth() && aBitmapSizePixel.getHeight())
                {
                    // create transform for the created bitmap in discrete coordinates first.
                    basegfx::B2DHomMatrix aNew2DTransform;

                    aNew2DTransform.set(0, 0, aVisibleDiscreteRange.getWidth());
                    aNew2DTransform.set(1, 1, aVisibleDiscreteRange.getHeight());
                    aNew2DTransform.set(0, 2, aVisibleDiscreteRange.getMinX());
                    aNew2DTransform.set(1, 2, aVisibleDiscreteRange.getMinY());

                    // transform back to world coordinates for usage in primitive creation
                    aNew2DTransform *= aInverseOToV;

                    // create bitmap primitive and add
                    const Primitive2DReference xRef(new BitmapPrimitive2D(maOldRenderedBitmap, aNew2DTransform));
                    appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, xRef);

                    // test: Allow to add an outline in the debugger when tests are needed
                    static bool bAddOutlineToCreated3DSceneRepresentation(false);

                    if(bAddOutlineToCreated3DSceneRepresentation)
                    {
                        basegfx::B2DPolygon aOutline(basegfx::tools::createUnitPolygon());
                        aOutline.transform(aNew2DTransform);
                        const Primitive2DReference xRef2(new PolygonHairlinePrimitive2D(aOutline, basegfx::BColor(1.0, 0.0, 0.0)));
                        appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, xRef2);
                    }
                }
            }

            return aRetval;
        }

        Primitive2DSequence ScenePrimitive2D::getGeometry2D() const
        {
            Primitive2DSequence aRetval;

            // create 2D projected geometry from 3D geometry
            if(getChildren3D().hasElements())
            {
                // create 2D geometry extraction processor
                processor3d::Geometry2DExtractingProcessor aGeometryProcessor(
                    getViewInformation3D(),
                    getObjectTransformation());

                // process local primitives
                aGeometryProcessor.process(getChildren3D());

                // fetch result
                aRetval = aGeometryProcessor.getPrimitive2DSequence();
            }

            return aRetval;
        }

        Primitive2DSequence ScenePrimitive2D::getShadow2D(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DSequence aRetval;

            // create 2D shadows from contained 3D primitives
            if(impGetShadow3D(rViewInformation))
            {
                // add extracted 2d shadows (before 3d scene creations itself)
                aRetval = maShadowPrimitives;
            }

            return aRetval;
        }

        bool ScenePrimitive2D::tryToCheckLastVisualisationDirectHit(const basegfx::B2DPoint& rLogicHitPoint, bool& o_rResult) const
        {
            if(!maOldRenderedBitmap.IsEmpty() && !maOldUnitVisiblePart.isEmpty())
            {
                basegfx::B2DHomMatrix aInverseSceneTransform(getObjectTransformation());
                aInverseSceneTransform.invert();
                const basegfx::B2DPoint aRelativePoint(aInverseSceneTransform * rLogicHitPoint);

                if(maOldUnitVisiblePart.isInside(aRelativePoint))
                {
                    // calculate coordinates relative to visualized part
                    double fDivisorX(maOldUnitVisiblePart.getWidth());
                    double fDivisorY(maOldUnitVisiblePart.getHeight());

                    if(basegfx::fTools::equalZero(fDivisorX))
                    {
                        fDivisorX = 1.0;
                    }

                    if(basegfx::fTools::equalZero(fDivisorY))
                    {
                        fDivisorY = 1.0;
                    }

                    const double fRelativeX((aRelativePoint.getX() - maOldUnitVisiblePart.getMinX()) / fDivisorX);
                    const double fRelativeY((aRelativePoint.getY() - maOldUnitVisiblePart.getMinY()) / fDivisorY);

                    // combine with real BitmapSizePixel to get bitmap coordinates
                    const Size aBitmapSizePixel(maOldRenderedBitmap.GetSizePixel());
                    const sal_Int32 nX(basegfx::fround(fRelativeX * aBitmapSizePixel.Width()));
                    const sal_Int32 nY(basegfx::fround(fRelativeY * aBitmapSizePixel.Height()));

                    // try to get a statement about transparency in that pixel
                    o_rResult = (0xff != maOldRenderedBitmap.GetTransparency(nX, nY));
                    return true;
                }
            }

            return false;
        }

        ScenePrimitive2D::ScenePrimitive2D(
            const primitive3d::Primitive3DSequence& rxChildren3D,
            const attribute::SdrSceneAttribute& rSdrSceneAttribute,
            const attribute::SdrLightingAttribute& rSdrLightingAttribute,
            const basegfx::B2DHomMatrix& rObjectTransformation,
            const geometry::ViewInformation3D& rViewInformation3D)
        :   BufferedDecompositionPrimitive2D(),
            mxChildren3D(rxChildren3D),
            maSdrSceneAttribute(rSdrSceneAttribute),
            maSdrLightingAttribute(rSdrLightingAttribute),
            maObjectTransformation(rObjectTransformation),
            maViewInformation3D(rViewInformation3D),
            maShadowPrimitives(),
            mbShadow3DChecked(false),
            mfOldDiscreteSizeX(0.0),
            mfOldDiscreteSizeY(0.0),
            maOldUnitVisiblePart(),
            maOldRenderedBitmap()
        {
        }

        bool ScenePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const ScenePrimitive2D& rCompare = (ScenePrimitive2D&)rPrimitive;

                return (primitive3d::arePrimitive3DSequencesEqual(getChildren3D(), rCompare.getChildren3D())
                    && getSdrSceneAttribute() == rCompare.getSdrSceneAttribute()
                    && getSdrLightingAttribute() == rCompare.getSdrLightingAttribute()
                    && getObjectTransformation() == rCompare.getObjectTransformation()
                    && getViewInformation3D() == rCompare.getViewInformation3D());
            }

            return false;
        }

        basegfx::B2DRange ScenePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            // transform unit range to discrete coordinate range
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
            aRetval.transform(rViewInformation.getObjectToViewTransformation() * getObjectTransformation());

            // force to discrete expanded bounds (it grows, so expanding works perfectly well)
            aRetval.expand(basegfx::B2DTuple(floor(aRetval.getMinX()), floor(aRetval.getMinY())));
            aRetval.expand(basegfx::B2DTuple(ceil(aRetval.getMaxX()), ceil(aRetval.getMaxY())));

            // transform back from discrete (view) to world coordinates
            aRetval.transform(rViewInformation.getInverseObjectToViewTransformation());

            // expand by evtl. existing shadow primitives
            if(impGetShadow3D(rViewInformation))
            {
                const basegfx::B2DRange aShadow2DRange(getB2DRangeFromPrimitive2DSequence(maShadowPrimitives, rViewInformation));

                if(!aShadow2DRange.isEmpty())
                {
                    aRetval.expand(aShadow2DRange);
                }
            }

            return aRetval;
        }

        Primitive2DSequence ScenePrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // get the involved ranges (see helper method calculateDiscreteSizes for details)
            basegfx::B2DRange aDiscreteRange;
            basegfx::B2DRange aUnitVisibleRange;
            bool bNeedNewDecomposition(false);
            bool bDiscreteSizesAreCalculated(false);

            if(getBuffered2DDecomposition().hasElements())
            {
                basegfx::B2DRange aVisibleDiscreteRange;
                calculateDiscreteSizes(rViewInformation, aDiscreteRange, aVisibleDiscreteRange, aUnitVisibleRange);
                bDiscreteSizesAreCalculated = true;

                // needs to be painted when the new part is not part of the last
                // decomposition
                if(!maOldUnitVisiblePart.isInside(aUnitVisibleRange))
                {
                    bNeedNewDecomposition = true;
                }

                // display has changed and cannot be reused when resolution got bigger. It
                // can be reused when resolution got smaller, though.
                if(!bNeedNewDecomposition)
                {
                    if(basegfx::fTools::more(aDiscreteRange.getWidth(), mfOldDiscreteSizeX) ||
                        basegfx::fTools::more(aDiscreteRange.getHeight(), mfOldDiscreteSizeY))
                    {
                        bNeedNewDecomposition = true;
                    }
                }
            }

            if(bNeedNewDecomposition)
            {
                // conditions of last local decomposition have changed, delete
                const_cast< ScenePrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
            }

            if(!getBuffered2DDecomposition().hasElements())
            {
                if(!bDiscreteSizesAreCalculated)
                {
                    basegfx::B2DRange aVisibleDiscreteRange;
                    calculateDiscreteSizes(rViewInformation, aDiscreteRange, aVisibleDiscreteRange, aUnitVisibleRange);
                }

                // remember last used NewDiscreteSize and NewUnitVisiblePart
                ScenePrimitive2D* pThat = const_cast< ScenePrimitive2D* >(this);
                pThat->mfOldDiscreteSizeX = aDiscreteRange.getWidth();
                pThat->mfOldDiscreteSizeY = aDiscreteRange.getHeight();
                pThat->maOldUnitVisiblePart = aUnitVisibleRange;
            }

            // use parent implementation
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(ScenePrimitive2D, PRIMITIVE2D_ID_SCENEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
