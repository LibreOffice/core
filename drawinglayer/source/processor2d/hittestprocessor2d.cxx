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

#include <drawinglayer/processor2d/hittestprocessor2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/sceneprimitive2d.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <drawinglayer/processor3d/cutfindprocessor3d.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        HitTestProcessor2D::HitTestProcessor2D(const geometry::ViewInformation2D& rViewInformation,
            const basegfx::B2DPoint& rLogicHitPosition,
            double fLogicHitTolerance,
            bool bHitTextOnly)
        :   BaseProcessor2D(rViewInformation),
            maDiscreteHitPosition(),
            mfDiscreteHitTolerance(0.0),
            mbHit(false),
            mbHitToleranceUsed(false),
            mbUseInvisiblePrimitiveContent(true),
            mbHitTextOnly(bHitTextOnly)
        {
            // init hit tolerance
            mfDiscreteHitTolerance = fLogicHitTolerance;

            if(basegfx::fTools::less(mfDiscreteHitTolerance, 0.0))
            {
                // ensure input parameter for hit tolerance is >= 0.0
                mfDiscreteHitTolerance = 0.0;
            }
            else if(basegfx::fTools::more(mfDiscreteHitTolerance, 0.0))
            {
                // generate discrete hit tolerance
                mfDiscreteHitTolerance = (getViewInformation2D().getObjectToViewTransformation()
                    * basegfx::B2DVector(mfDiscreteHitTolerance, 0.0)).getLength();
            }

            // gererate discrete hit position
            maDiscreteHitPosition = getViewInformation2D().getObjectToViewTransformation() * rLogicHitPosition;

            // check if HitTolerance is used
            mbHitToleranceUsed = basegfx::fTools::more(getDiscreteHitTolerance(), 0.0);
        }

        HitTestProcessor2D::~HitTestProcessor2D()
        {
        }

        bool HitTestProcessor2D::checkHairlineHitWithTolerance(
            const basegfx::B2DPolygon& rPolygon,
            double fDiscreteHitTolerance)
        {
            basegfx::B2DPolygon aLocalPolygon(rPolygon);
            aLocalPolygon.transform(getViewInformation2D().getObjectToViewTransformation());

            // get discrete range
            basegfx::B2DRange aPolygonRange(aLocalPolygon.getB2DRange());

            if(basegfx::fTools::more(fDiscreteHitTolerance, 0.0))
            {
                aPolygonRange.grow(fDiscreteHitTolerance);
            }

            // do rough range test first
            if(aPolygonRange.isInside(getDiscreteHitPosition()))
            {
                // check if a polygon edge is hit
                return basegfx::tools::isInEpsilonRange(
                    aLocalPolygon,
                    getDiscreteHitPosition(),
                    fDiscreteHitTolerance);
            }

            return false;
        }

        bool HitTestProcessor2D::checkFillHitWithTolerance(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            double fDiscreteHitTolerance)
        {
            bool bRetval(false);
            basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolyPolygon);
            aLocalPolyPolygon.transform(getViewInformation2D().getObjectToViewTransformation());

            // get discrete range
            basegfx::B2DRange aPolygonRange(aLocalPolyPolygon.getB2DRange());
            const bool bDiscreteHitToleranceUsed(basegfx::fTools::more(fDiscreteHitTolerance, 0.0));

            if(bDiscreteHitToleranceUsed)
            {
                aPolygonRange.grow(fDiscreteHitTolerance);
            }

            // do rough range test first
            if(aPolygonRange.isInside(getDiscreteHitPosition()))
            {
                // if a HitTolerance is given, check for polygon edge hit in epsilon first
                if(bDiscreteHitToleranceUsed &&
                    basegfx::tools::isInEpsilonRange(
                        aLocalPolyPolygon,
                        getDiscreteHitPosition(),
                        fDiscreteHitTolerance))
                {
                    bRetval = true;
                }

                // check for hit in filled polyPolygon
                if(!bRetval && basegfx::tools::isInside(
                    aLocalPolyPolygon,
                    getDiscreteHitPosition(),
                    true))
                {
                    bRetval = true;
                }
            }

            return bRetval;
        }

        void HitTestProcessor2D::check3DHit(const primitive2d::ScenePrimitive2D& rCandidate)
        {
            // calculate relative point in unified 2D scene
            const basegfx::B2DPoint aLogicHitPosition(getViewInformation2D().getInverseObjectToViewTransformation() * getDiscreteHitPosition());

            // use bitmap check in ScenePrimitive2D
            bool bTryFastResult(false);

            if(rCandidate.tryToCheckLastVisualisationDirectHit(aLogicHitPosition, bTryFastResult))
            {
                mbHit = bTryFastResult;
            }
            else
            {
                basegfx::B2DHomMatrix aInverseSceneTransform(rCandidate.getObjectTransformation());
                aInverseSceneTransform.invert();
                const basegfx::B2DPoint aRelativePoint(aInverseSceneTransform * aLogicHitPosition);

                // check if test point is inside scene's unified area at all
                if(aRelativePoint.getX() >= 0.0 && aRelativePoint.getX() <= 1.0
                    && aRelativePoint.getY() >= 0.0 && aRelativePoint.getY() <= 1.0)
                {
                    // get 3D view information
                    const geometry::ViewInformation3D& rObjectViewInformation3D = rCandidate.getViewInformation3D();

                    // create HitPoint Front and Back, transform to object coordinates
                    basegfx::B3DHomMatrix aViewToObject(rObjectViewInformation3D.getObjectToView());
                    aViewToObject.invert();
                    const basegfx::B3DPoint aFront(aViewToObject * basegfx::B3DPoint(aRelativePoint.getX(), aRelativePoint.getY(), 0.0));
                    const basegfx::B3DPoint aBack(aViewToObject * basegfx::B3DPoint(aRelativePoint.getX(), aRelativePoint.getY(), 1.0));

                    if(!aFront.equal(aBack))
                    {
                        const primitive3d::Primitive3DSequence& rPrimitives = rCandidate.getChildren3D();

                        if(rPrimitives.hasElements())
                        {
                            // make BoundVolume empty and overlapping test for speedup
                            const basegfx::B3DRange aObjectRange(
                                drawinglayer::primitive3d::getB3DRangeFromPrimitive3DSequence(
                                    rPrimitives, rObjectViewInformation3D));

                            if(!aObjectRange.isEmpty())
                            {
                                const basegfx::B3DRange aFrontBackRange(aFront, aBack);

                                if(aObjectRange.overlaps(aFrontBackRange))
                                {
                                    // bound volumes hit, geometric cut tests needed
                                    drawinglayer::processor3d::CutFindProcessor aCutFindProcessor(
                                        rObjectViewInformation3D,
                                        aFront,
                                        aBack,
                                        true);
                                    aCutFindProcessor.process(rPrimitives);

                                    mbHit = (0 != aCutFindProcessor.getCutPoints().size());
                                }
                            }
                        }
                    }
                }

                // This is needed to check hit with 3D shadows, too. HitTest is without shadow
                // to keep compatible with previous versions. Keeping here as reference
                //
                // if(!getHit())
                // {
                //     // if scene has shadow, check hit with shadow, too
                //     const primitive2d::Primitive2DSequence xExtracted2DSceneShadow(rCandidate.getShadow2D(getViewInformation2D()));
                //
                //     if(xExtracted2DSceneShadow.hasElements())
                //     {
                //         // proccess extracted 2D content
                //         process(xExtracted2DSceneShadow);
                //     }
                // }

                if(!getHit())
                {
                    // empty 3D scene; Check for border hit
                    basegfx::B2DPolygon aOutline(basegfx::tools::createUnitPolygon());
                    aOutline.transform(rCandidate.getObjectTransformation());

                    mbHit = checkHairlineHitWithTolerance(aOutline, getDiscreteHitTolerance());
                }

                // This is what the previous version did. Keeping it here for reference
                //
                // // 2D Scene primitive containing 3D stuff; extract 2D contour in world coordinates
                // // This may be refined later to an own 3D HitTest renderer which processes the 3D
                // // geometry directly
                // const primitive2d::ScenePrimitive2D& rScenePrimitive2DCandidate(static_cast< const primitive2d::ScenePrimitive2D& >(rCandidate));
                // const primitive2d::Primitive2DSequence xExtracted2DSceneGeometry(rScenePrimitive2DCandidate.getGeometry2D());
                // const primitive2d::Primitive2DSequence xExtracted2DSceneShadow(rScenePrimitive2DCandidate.getShadow2D(getViewInformation2D()));
                //
                // if(xExtracted2DSceneGeometry.hasElements() || xExtracted2DSceneShadow.hasElements())
                // {
                //     // proccess extracted 2D content
                //     process(xExtracted2DSceneGeometry);
                //     process(xExtracted2DSceneShadow);
                // }
                // else
                // {
                //     // empty 3D scene; Check for border hit
                //     const basegfx::B2DRange aRange(rCandidate.getB2DRange(getViewInformation2D()));
                //     if(!aRange.isEmpty())
                //     {
                //          const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(aRange));
                //          mbHit = checkHairlineHitWithTolerance(aOutline, getDiscreteHitTolerance());
                //     }
                // }
            }
        }

        void HitTestProcessor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
        {
            if(getHit())
            {
                // stop processing as soon as a hit was recognized
                return;
            }

            switch(rCandidate.getPrimitive2DID())
            {
                case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D :
                {
                    // remember current ViewInformation2D
                    const primitive2d::TransformPrimitive2D& rTransformCandidate(static_cast< const primitive2d::TransformPrimitive2D& >(rCandidate));
                    const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

                    // create new local ViewInformation2D containing transformation
                    const geometry::ViewInformation2D aViewInformation2D(
                        getViewInformation2D().getObjectTransformation() * rTransformCandidate.getTransformation(),
                        getViewInformation2D().getViewTransformation(),
                        getViewInformation2D().getViewport(),
                        getViewInformation2D().getVisualizedPage(),
                        getViewInformation2D().getViewTime(),
                        getViewInformation2D().getExtendedInformationSequence());
                    updateViewInformation(aViewInformation2D);

                    // proccess child content recursively
                    process(rTransformCandidate.getChildren());

                    // restore transformations
                    updateViewInformation(aLastViewInformation2D);

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                {
                    if(!getHitTextOnly())
                    {
                        // create hairline in discrete coordinates
                        const primitive2d::PolygonHairlinePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate));

                        // use hairline test
                        mbHit = checkHairlineHitWithTolerance(rPolygonCandidate.getB2DPolygon(), getDiscreteHitTolerance());
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONMARKERPRIMITIVE2D :
                {
                    if(!getHitTextOnly())
                    {
                        // handle marker like hairline; no need to decompose in dashes
                        const primitive2d::PolygonMarkerPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolygonMarkerPrimitive2D& >(rCandidate));

                        // use hairline test
                        mbHit = checkHairlineHitWithTolerance(rPolygonCandidate.getB2DPolygon(), getDiscreteHitTolerance());
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D :
                {
                    if(!getHitTextOnly())
                    {
                        // handle stroke evtl. directly; no need to decompose to filled polygon outlines
                        const primitive2d::PolygonStrokePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolygonStrokePrimitive2D& >(rCandidate));
                        const attribute::LineAttribute& rLineAttribute = rPolygonCandidate.getLineAttribute();

                        if(basegfx::fTools::more(rLineAttribute.getWidth(), 0.0))
                        {
                            if(basegfx::B2DLINEJOIN_MITER == rLineAttribute.getLineJoin())
                            {
                                // if line is mitered, use decomposition since mitered line
                                // geometry may use more space than the geometry grown by half line width
                                process(rCandidate.get2DDecomposition(getViewInformation2D()));
                            }
                            else
                            {
                                // for all other B2DLINEJOIN_* do a hairline HitTest with expanded tolerance
                                const basegfx::B2DVector aDiscreteHalfLineVector(getViewInformation2D().getObjectToViewTransformation()
                                    * basegfx::B2DVector(rLineAttribute.getWidth() * 0.5, 0.0));
                                mbHit = checkHairlineHitWithTolerance(
                                    rPolygonCandidate.getB2DPolygon(),
                                    getDiscreteHitTolerance() + aDiscreteHalfLineVector.getLength());
                            }
                        }
                        else
                        {
                            // hairline; fallback to hairline test. Do not decompose
                            // since this may decompose the hairline to dashes
                            mbHit = checkHairlineHitWithTolerance(rPolygonCandidate.getB2DPolygon(), getDiscreteHitTolerance());
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONWAVEPRIMITIVE2D :
                {
                    if(!getHitTextOnly())
                    {
                        // do not use decompose; just handle like a line with width
                        const primitive2d::PolygonWavePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolygonWavePrimitive2D& >(rCandidate));
                        double fLogicHitTolerance(0.0);

                        // if WaveHeight, grow by it
                        if(basegfx::fTools::more(rPolygonCandidate.getWaveHeight(), 0.0))
                        {
                            fLogicHitTolerance += rPolygonCandidate.getWaveHeight();
                        }

                        // if line width, grow by it
                        if(basegfx::fTools::more(rPolygonCandidate.getLineAttribute().getWidth(), 0.0))
                        {
                            fLogicHitTolerance += rPolygonCandidate.getLineAttribute().getWidth() * 0.5;
                        }

                        const basegfx::B2DVector aDiscreteHalfLineVector(getViewInformation2D().getObjectToViewTransformation()
                            * basegfx::B2DVector(fLogicHitTolerance, 0.0));

                        mbHit = checkHairlineHitWithTolerance(
                            rPolygonCandidate.getB2DPolygon(),
                            getDiscreteHitTolerance() + aDiscreteHalfLineVector.getLength());
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D :
                {
                    if(!getHitTextOnly())
                    {
                        // create filled polyPolygon in discrete coordinates
                        const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolyPolygonColorPrimitive2D& >(rCandidate));

                        // use fill hit test
                        mbHit = checkFillHitWithTolerance(rPolygonCandidate.getB2DPolyPolygon(), getDiscreteHitTolerance());
                    }

                    break;
                }
                case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D :
                {
                    // sub-transparence group
                    const primitive2d::TransparencePrimitive2D& rTransCandidate(static_cast< const primitive2d::TransparencePrimitive2D& >(rCandidate));

                    // Currently the transparence content is not taken into account; only
                    // the children are recursively checked for hit. This may be refined for
                    // parts where the content is completely transparent if needed.
                    process(rTransCandidate.getChildren());

                    break;
                }
                case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
                {
                    // create mask in discrete coordinates; only recursively continue
                    // with content when HitTest position is inside the mask
                    const primitive2d::MaskPrimitive2D& rMaskCandidate(static_cast< const primitive2d::MaskPrimitive2D& >(rCandidate));

                    // use fill hit test
                    if(checkFillHitWithTolerance(rMaskCandidate.getMask(), getDiscreteHitTolerance()))
                    {
                        // recursively HitTest children
                        process(rMaskCandidate.getChildren());
                    }

                    break;
                }
                case PRIMITIVE2D_ID_SCENEPRIMITIVE2D :
                {
                    if(!getHitTextOnly())
                    {
                        const primitive2d::ScenePrimitive2D& rScenePrimitive2D(
                            static_cast< const primitive2d::ScenePrimitive2D& >(rCandidate));
                        check3DHit(rScenePrimitive2D);
                    }

                    break;
                }
                case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D :
                case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D :
                case PRIMITIVE2D_ID_GRIDPRIMITIVE2D :
                case PRIMITIVE2D_ID_HELPLINEPRIMITIVE2D :
                {
                    // ignorable primitives
                    break;
                }
                case PRIMITIVE2D_ID_SHADOWPRIMITIVE2D :
                {
                    // Ignore shadows; we do not want to have shadows hittable.
                    // Remove this one to make shadows hittable on demand.
                    break;
                }
                case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D :
                case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D :
                {
                    // for text use the BoundRect of the primitive itself
                    const basegfx::B2DRange aRange(rCandidate.getB2DRange(getViewInformation2D()));

                    if(!aRange.isEmpty())
                    {
                        const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(aRange));
                        mbHit = checkFillHitWithTolerance(basegfx::B2DPolyPolygon(aOutline), getDiscreteHitTolerance());
                    }

                    break;
                }
                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                {
                    if(!getHitTextOnly())
                    {
                        // The recently added BitmapEx::GetTransparency() makes it easy to extend
                        // the BitmapPrimitive2D HitTest to take the contained BotmapEx and it's
                        // transparency into account
                        const basegfx::B2DRange aRange(rCandidate.getB2DRange(getViewInformation2D()));

                        if(!aRange.isEmpty())
                        {
                            const primitive2d::BitmapPrimitive2D& rBitmapCandidate(static_cast< const primitive2d::BitmapPrimitive2D& >(rCandidate));
                            const BitmapEx& rBitmapEx = rBitmapCandidate.getBitmapEx();
                            const Size& rSizePixel(rBitmapEx.GetSizePixel());

                            if(rSizePixel.Width() && rSizePixel.Height())
                            {
                                basegfx::B2DHomMatrix aBackTransform(
                                    getViewInformation2D().getObjectToViewTransformation() *
                                    rBitmapCandidate.getTransform());
                                aBackTransform.invert();

                                const basegfx::B2DPoint aRelativePoint(aBackTransform * getDiscreteHitPosition());
                                const basegfx::B2DRange aUnitRange(0.0, 0.0, 1.0, 1.0);

                                if(aUnitRange.isInside(aRelativePoint))
                                {
                                    const sal_Int32 nX(basegfx::fround(aRelativePoint.getX() * rSizePixel.Width()));
                                    const sal_Int32 nY(basegfx::fround(aRelativePoint.getY() * rSizePixel.Height()));

                                    mbHit = (0xff != rBitmapEx.GetTransparency(nX, nY));
                                }
                            }
                            else
                            {
                                // fallback to standard HitTest
                                const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(aRange));
                                mbHit = checkFillHitWithTolerance(basegfx::B2DPolyPolygon(aOutline), getDiscreteHitTolerance());
                            }
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D :
                case PRIMITIVE2D_ID_CONTROLPRIMITIVE2D :
                case PRIMITIVE2D_ID_FILLGRADIENTPRIMITIVE2D :
                case PRIMITIVE2D_ID_FILLHATCHPRIMITIVE2D :
                case PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D :
                case PRIMITIVE2D_ID_MEDIAPRIMITIVE2D:
                case PRIMITIVE2D_ID_RENDERGRAPHICPRIMITIVE2D:
                {
                    if(!getHitTextOnly())
                    {
                        // Class of primitives for which just the BoundRect of the primitive itself
                        // will be used for HitTest currently.
                        //
                        // This may be refined in the future, e.g:
                        // - For Bitamps, the mask and/or transparence information may be used
                        // - For MetaFiles, the MetaFile content may be used
                        const basegfx::B2DRange aRange(rCandidate.getB2DRange(getViewInformation2D()));

                        if(!aRange.isEmpty())
                        {
                            const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(aRange));
                            mbHit = checkFillHitWithTolerance(basegfx::B2DPolyPolygon(aOutline), getDiscreteHitTolerance());
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_HIDDENGEOMETRYPRIMITIVE2D :
                {
                    // HiddenGeometryPrimitive2D; the default decomposition would return an empty seqence,
                    // so force this primitive to process it's children directly if the switch is set
                    // (which is the default). Else, ignore invisible content
                    const primitive2d::HiddenGeometryPrimitive2D& rHiddenGeometry(static_cast< const primitive2d::HiddenGeometryPrimitive2D& >(rCandidate));
                       const primitive2d::Primitive2DSequence& rChildren = rHiddenGeometry.getChildren();

                    if(rChildren.hasElements())
                    {
                        if(getUseInvisiblePrimitiveContent())
                        {
                            process(rChildren);
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D :
                {
                    if(!getHitTextOnly())
                    {
                        const primitive2d::PointArrayPrimitive2D& rPointArrayCandidate(static_cast< const primitive2d::PointArrayPrimitive2D& >(rCandidate));
                        const std::vector< basegfx::B2DPoint >& rPositions = rPointArrayCandidate.getPositions();
                        const sal_uInt32 nCount(rPositions.size());

                        for(sal_uInt32 a(0); !getHit() && a < nCount; a++)
                        {
                            const basegfx::B2DPoint aPosition(getViewInformation2D().getObjectToViewTransformation() * rPositions[a]);
                            const basegfx::B2DVector aDistance(aPosition - getDiscreteHitPosition());

                            if(aDistance.getLength() <= getDiscreteHitTolerance())
                            {
                                mbHit = true;
                            }
                        }
                    }

                    break;
                }
                default :
                {
                    // process recursively
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));

                    break;
                }
            }
        }

    } // end of namespace processor2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
