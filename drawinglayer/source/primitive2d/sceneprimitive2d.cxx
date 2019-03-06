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
#include <basegfx/utils/canvastools.hxx>
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
#include <basegfx/raster/bzpixelraster.hxx>
#include <vcl/BitmapTools.hxx>
#include <comphelper/threadpool.hxx>

using namespace com::sun::star;

namespace
{
    BitmapEx BPixelRasterToBitmapEx(const basegfx::BZPixelRaster& rRaster, sal_uInt16 mnAntiAlialize)
    {
        BitmapEx aRetval;
        const sal_uInt32 nWidth(mnAntiAlialize ? rRaster.getWidth()/mnAntiAlialize : rRaster.getWidth());
        const sal_uInt32 nHeight(mnAntiAlialize ? rRaster.getHeight()/mnAntiAlialize : rRaster.getHeight());

        if(nWidth && nHeight)
        {
            const Size aDestSize(nWidth, nHeight);
            vcl::bitmap::RawBitmap aContent(aDestSize, 32);

            if(mnAntiAlialize)
            {
                const sal_uInt16 nDivisor(mnAntiAlialize * mnAntiAlialize);

                for(sal_uInt32 y(0); y < nHeight; y++)
                {
                    for(sal_uInt32 x(0); x < nWidth; x++)
                    {
                        sal_uInt16 nRed(0);
                        sal_uInt16 nGreen(0);
                        sal_uInt16 nBlue(0);
                        sal_uInt16 nOpacity(0);
                        sal_uInt32 nIndex(rRaster.getIndexFromXY(x * mnAntiAlialize, y * mnAntiAlialize));

                        for(sal_uInt32 c(0); c < mnAntiAlialize; c++)
                        {
                            for(sal_uInt32 d(0); d < mnAntiAlialize; d++)
                            {
                                const basegfx::BPixel& rPixel(rRaster.getBPixel(nIndex++));
                                nRed = nRed + rPixel.getRed();
                                nGreen = nGreen + rPixel.getGreen();
                                nBlue = nBlue + rPixel.getBlue();
                                nOpacity = nOpacity + rPixel.getOpacity();
                            }

                            nIndex += rRaster.getWidth() - mnAntiAlialize;
                        }

                        nOpacity = nOpacity / nDivisor;

                        if(nOpacity)
                        {
                            aContent.SetPixel(y, x, Color(
                                255 - static_cast<sal_uInt8>(nOpacity),
                                static_cast<sal_uInt8>(nRed / nDivisor),
                                static_cast<sal_uInt8>(nGreen / nDivisor),
                                static_cast<sal_uInt8>(nBlue / nDivisor) ));
                        }
                        else
                            aContent.SetPixel(y, x, Color(255, 0, 0, 0));
                    }
                }
            }
            else
            {
                sal_uInt32 nIndex(0);

                for(sal_uInt32 y(0); y < nHeight; y++)
                {
                    for(sal_uInt32 x(0); x < nWidth; x++)
                    {
                        const basegfx::BPixel& rPixel(rRaster.getBPixel(nIndex++));

                        if(rPixel.getOpacity())
                        {
                            aContent.SetPixel(y, x, Color(255 - rPixel.getOpacity(), rPixel.getRed(), rPixel.getGreen(), rPixel.getBlue()));
                        }
                        else
                            aContent.SetPixel(y, x, Color(255, 0, 0, 0));
                    }
                }
            }

            aRetval = vcl::bitmap::CreateFromData(std::move(aContent));

            // #i101811# set PrefMapMode and PrefSize at newly created Bitmap
            aRetval.SetPrefMapMode(MapMode(MapUnit::MapPixel));
            aRetval.SetPrefSize(Size(nWidth, nHeight));
        }

        return aRetval;
    }
} // end of anonymous namespace

namespace drawinglayer
{
    namespace primitive2d
    {
        bool ScenePrimitive2D::impGetShadow3D() const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // create on demand
            if(!mbShadow3DChecked && !getChildren3D().empty())
            {
                basegfx::B3DVector aLightNormal;
                const double fShadowSlant(getSdrSceneAttribute().getShadowSlant());
                const basegfx::B3DRange aScene3DRange(getChildren3D().getB3DRange(getViewInformation3D()));

                if(!maSdrLightingAttribute.getLightVector().empty())
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
            return !maShadowPrimitives.empty();
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

        void ScenePrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const
        {
            // create 2D shadows from contained 3D primitives. This creates the shadow primitives on demand and tells if
            // there are some or not. Do this at start, the shadow might still be visible even when the scene is not
            if(impGetShadow3D())
            {
                // test visibility
                const basegfx::B2DRange aShadow2DRange(maShadowPrimitives.getB2DRange(rViewInformation));
                const basegfx::B2DRange aViewRange(
                    rViewInformation.getViewport());

                if(aViewRange.isEmpty() || aShadow2DRange.overlaps(aViewRange))
                {
                    // add extracted 2d shadows (before 3d scene creations itself)
                    rContainer.insert(rContainer.end(), maShadowPrimitives.begin(), maShadowPrimitives.end());
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
                static const sal_uInt16 nDefaultOversampleValue(3);
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

                        // bring to unit coordinates, flip Y, leave Z unchanged
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
                const basegfx::B2DHomMatrix& aInverseOToV(rViewInformation.getInverseObjectToViewTransformation());
                const double fLogicX((aInverseOToV * basegfx::B2DVector(aDiscreteRange.getWidth() * fReduceFactor, 0.0)).getLength());
                const double fLogicY((aInverseOToV * basegfx::B2DVector(0.0, aDiscreteRange.getHeight() * fReduceFactor)).getLength());

                // generate ViewSizes
                const double fFullViewSizeX((rViewInformation.getObjectToViewTransformation() * basegfx::B2DVector(fLogicX, 0.0)).getLength());
                const double fFullViewSizeY((rViewInformation.getObjectToViewTransformation() * basegfx::B2DVector(0.0, fLogicY)).getLength());

                // generate RasterWidth and RasterHeight for visible part
                const sal_Int32 nRasterWidth(basegfx::fround(fFullViewSizeX * aUnitVisibleRange.getWidth()) + 1);
                const sal_Int32 nRasterHeight(basegfx::fround(fFullViewSizeY * aUnitVisibleRange.getHeight()) + 1);

                if(nRasterWidth && nRasterHeight)
                {
                    // create view unit buffer
                    basegfx::BZPixelRaster aBZPixelRaster(
                        nOversampleValue ? nRasterWidth * nOversampleValue : nRasterWidth,
                        nOversampleValue ? nRasterHeight * nOversampleValue : nRasterHeight);

                    // check for parallel execution possibilities
                    static bool bMultithreadAllowed = true; // loplugin:constvars:ignore
                    sal_Int32 nThreadCount(0);
                    comphelper::ThreadPool& rThreadPool(comphelper::ThreadPool::getSharedOptimalPool());

                    if(bMultithreadAllowed)
                    {
                        nThreadCount = rThreadPool.getWorkerCount();

                        if(nThreadCount > 1)
                        {
                            // at least use 10px per processor, so limit number of processors to
                            // target pixel size divided by 10 (which might be zero what is okay)
                            nThreadCount = std::min(nThreadCount, nRasterHeight / 10);
                        }
                    }

                    if(nThreadCount > 1)
                    {
                        class Executor : public comphelper::ThreadTask
                        {
                        private:
                            std::unique_ptr<processor3d::ZBufferProcessor3D> mpZBufferProcessor3D;
                            const primitive3d::Primitive3DContainer&    mrChildren3D;

                        public:
                            explicit Executor(
                                std::shared_ptr<comphelper::ThreadTaskTag> const & rTag,
                                std::unique_ptr<processor3d::ZBufferProcessor3D> pZBufferProcessor3D,
                                const primitive3d::Primitive3DContainer& rChildren3D)
                            :   comphelper::ThreadTask(rTag),
                                mpZBufferProcessor3D(std::move(pZBufferProcessor3D)),
                                mrChildren3D(rChildren3D)
                            {
                            }

                            virtual void doWork() override
                            {
                                mpZBufferProcessor3D->process(mrChildren3D);
                                mpZBufferProcessor3D->finish();
                                mpZBufferProcessor3D.reset();
                            }
                        };

                        const sal_uInt32 nLinesPerThread(aBZPixelRaster.getHeight() / nThreadCount);
                        std::shared_ptr<comphelper::ThreadTaskTag> aTag = comphelper::ThreadPool::createThreadTaskTag();

                        for(sal_Int32 a(0); a < nThreadCount; a++)
                        {
                            std::unique_ptr<processor3d::ZBufferProcessor3D> pNewZBufferProcessor3D(new processor3d::ZBufferProcessor3D(
                                aViewInformation3D,
                                getSdrSceneAttribute(),
                                getSdrLightingAttribute(),
                                aUnitVisibleRange,
                                nOversampleValue,
                                fFullViewSizeX,
                                fFullViewSizeY,
                                aBZPixelRaster,
                                nLinesPerThread * a,
                                a + 1 == nThreadCount ? aBZPixelRaster.getHeight() : nLinesPerThread * (a + 1)));
                            std::unique_ptr<Executor> pExecutor(new Executor(aTag, std::move(pNewZBufferProcessor3D), getChildren3D()));
                            rThreadPool.pushTask(std::move(pExecutor));
                        }

                        rThreadPool.waitUntilDone(aTag);
                    }
                    else
                    {
                        // use default 3D primitive processor to create BitmapEx for aUnitVisiblePart and process
                        processor3d::ZBufferProcessor3D aZBufferProcessor3D(
                            aViewInformation3D,
                            getSdrSceneAttribute(),
                            getSdrLightingAttribute(),
                            aUnitVisibleRange,
                            nOversampleValue,
                            fFullViewSizeX,
                            fFullViewSizeY,
                            aBZPixelRaster,
                            0,
                            aBZPixelRaster.getHeight());

                        aZBufferProcessor3D.process(getChildren3D());
                        aZBufferProcessor3D.finish();
                    }

                    const_cast< ScenePrimitive2D* >(this)->maOldRenderedBitmap = BPixelRasterToBitmapEx(aBZPixelRaster, nOversampleValue);
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
                        rContainer.push_back(new BitmapPrimitive2D(maOldRenderedBitmap, aNew2DTransform));

                        // test: Allow to add an outline in the debugger when tests are needed
                        static bool bAddOutlineToCreated3DSceneRepresentation(false); // loplugin:constvars:ignore

                        if(bAddOutlineToCreated3DSceneRepresentation)
                        {
                            basegfx::B2DPolygon aOutline(basegfx::utils::createUnitPolygon());
                            aOutline.transform(aNew2DTransform);
                            rContainer.push_back(new PolygonHairlinePrimitive2D(aOutline, basegfx::BColor(1.0, 0.0, 0.0)));
                        }
                    }
                }
            }
        }

        Primitive2DContainer ScenePrimitive2D::getGeometry2D() const
        {
            Primitive2DContainer aRetval;

            // create 2D projected geometry from 3D geometry
            if(!getChildren3D().empty())
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

        Primitive2DContainer ScenePrimitive2D::getShadow2D() const
        {
            Primitive2DContainer aRetval;

            // create 2D shadows from contained 3D primitives
            if(impGetShadow3D())
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
            const primitive3d::Primitive3DContainer& rxChildren3D,
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
                const ScenePrimitive2D& rCompare = static_cast<const ScenePrimitive2D&>(rPrimitive);

                return (getChildren3D() == rCompare.getChildren3D()
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
            if(impGetShadow3D())
            {
                const basegfx::B2DRange aShadow2DRange(maShadowPrimitives.getB2DRange(rViewInformation));

                if(!aShadow2DRange.isEmpty())
                {
                    aRetval.expand(aShadow2DRange);
                }
            }

            return aRetval;
        }

        void ScenePrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // get the involved ranges (see helper method calculateDiscreteSizes for details)
            basegfx::B2DRange aDiscreteRange;
            basegfx::B2DRange aUnitVisibleRange;
            bool bNeedNewDecomposition(false);
            bool bDiscreteSizesAreCalculated(false);

            if(!getBuffered2DDecomposition().empty())
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
                const_cast< ScenePrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DContainer());
            }

            if(getBuffered2DDecomposition().empty())
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
            BufferedDecompositionPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(ScenePrimitive2D, PRIMITIVE2D_ID_SCENEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
