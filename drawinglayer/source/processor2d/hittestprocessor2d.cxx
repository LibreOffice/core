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
            
            mfDiscreteHitTolerance = fLogicHitTolerance;

            if(basegfx::fTools::less(mfDiscreteHitTolerance, 0.0))
            {
                
                mfDiscreteHitTolerance = 0.0;
            }
            else if(basegfx::fTools::more(mfDiscreteHitTolerance, 0.0))
            {
                
                mfDiscreteHitTolerance = (getViewInformation2D().getObjectToViewTransformation()
                    * basegfx::B2DVector(mfDiscreteHitTolerance, 0.0)).getLength();
            }

            
            maDiscreteHitPosition = getViewInformation2D().getObjectToViewTransformation() * rLogicHitPosition;

            
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

            
            basegfx::B2DRange aPolygonRange(aLocalPolygon.getB2DRange());

            if(basegfx::fTools::more(fDiscreteHitTolerance, 0.0))
            {
                aPolygonRange.grow(fDiscreteHitTolerance);
            }

            
            if(aPolygonRange.isInside(getDiscreteHitPosition()))
            {
                
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

            
            basegfx::B2DRange aPolygonRange(aLocalPolyPolygon.getB2DRange());
            const bool bDiscreteHitToleranceUsed(basegfx::fTools::more(fDiscreteHitTolerance, 0.0));

            if(bDiscreteHitToleranceUsed)
            {
                aPolygonRange.grow(fDiscreteHitTolerance);
            }

            
            if(aPolygonRange.isInside(getDiscreteHitPosition()))
            {
                
                if(bDiscreteHitToleranceUsed &&
                    basegfx::tools::isInEpsilonRange(
                        aLocalPolyPolygon,
                        getDiscreteHitPosition(),
                        fDiscreteHitTolerance))
                {
                    bRetval = true;
                }

                
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
            
            const basegfx::B2DPoint aLogicHitPosition(getViewInformation2D().getInverseObjectToViewTransformation() * getDiscreteHitPosition());

            
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

                
                if(aRelativePoint.getX() >= 0.0 && aRelativePoint.getX() <= 1.0
                    && aRelativePoint.getY() >= 0.0 && aRelativePoint.getY() <= 1.0)
                {
                    
                    const geometry::ViewInformation3D& rObjectViewInformation3D = rCandidate.getViewInformation3D();

                    
                    basegfx::B3DHomMatrix aViewToObject(rObjectViewInformation3D.getObjectToView());
                    aViewToObject.invert();
                    const basegfx::B3DPoint aFront(aViewToObject * basegfx::B3DPoint(aRelativePoint.getX(), aRelativePoint.getY(), 0.0));
                    const basegfx::B3DPoint aBack(aViewToObject * basegfx::B3DPoint(aRelativePoint.getX(), aRelativePoint.getY(), 1.0));

                    if(!aFront.equal(aBack))
                    {
                        const primitive3d::Primitive3DSequence& rPrimitives = rCandidate.getChildren3D();

                        if(rPrimitives.hasElements())
                        {
                            
                            const basegfx::B3DRange aObjectRange(
                                drawinglayer::primitive3d::getB3DRangeFromPrimitive3DSequence(
                                    rPrimitives, rObjectViewInformation3D));

                            if(!aObjectRange.isEmpty())
                            {
                                const basegfx::B3DRange aFrontBackRange(aFront, aBack);

                                if(aObjectRange.overlaps(aFrontBackRange))
                                {
                                    
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

                
                
                //
                
                
                
                
                //
                
                
                
                
                
                

                if(!getHit())
                {
                    
                    basegfx::B2DPolygon aOutline(basegfx::tools::createUnitPolygon());
                    aOutline.transform(rCandidate.getObjectTransformation());

                    mbHit = checkHairlineHitWithTolerance(aOutline, getDiscreteHitTolerance());
                }

                
                //
                
                
                
                
                
                
                //
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
            }
        }

        void HitTestProcessor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
        {
            if(getHit())
            {
                
                return;
            }

            switch(rCandidate.getPrimitive2DID())
            {
                case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D :
                {
                    
                    const primitive2d::TransformPrimitive2D& rTransformCandidate(static_cast< const primitive2d::TransformPrimitive2D& >(rCandidate));
                    const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

                    
                    const geometry::ViewInformation2D aViewInformation2D(
                        getViewInformation2D().getObjectTransformation() * rTransformCandidate.getTransformation(),
                        getViewInformation2D().getViewTransformation(),
                        getViewInformation2D().getViewport(),
                        getViewInformation2D().getVisualizedPage(),
                        getViewInformation2D().getViewTime(),
                        getViewInformation2D().getExtendedInformationSequence());
                    updateViewInformation(aViewInformation2D);

                    
                    process(rTransformCandidate.getChildren());

                    
                    updateViewInformation(aLastViewInformation2D);

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                {
                    if(!getHitTextOnly())
                    {
                        
                        const primitive2d::PolygonHairlinePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate));

                        
                        mbHit = checkHairlineHitWithTolerance(rPolygonCandidate.getB2DPolygon(), getDiscreteHitTolerance());
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONMARKERPRIMITIVE2D :
                {
                    if(!getHitTextOnly())
                    {
                        
                        const primitive2d::PolygonMarkerPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolygonMarkerPrimitive2D& >(rCandidate));

                        
                        mbHit = checkHairlineHitWithTolerance(rPolygonCandidate.getB2DPolygon(), getDiscreteHitTolerance());
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D :
                {
                    if(!getHitTextOnly())
                    {
                        
                        const primitive2d::PolygonStrokePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolygonStrokePrimitive2D& >(rCandidate));
                        const attribute::LineAttribute& rLineAttribute = rPolygonCandidate.getLineAttribute();

                        if(basegfx::fTools::more(rLineAttribute.getWidth(), 0.0))
                        {
                            if(basegfx::B2DLINEJOIN_MITER == rLineAttribute.getLineJoin())
                            {
                                
                                
                                process(rCandidate.get2DDecomposition(getViewInformation2D()));
                            }
                            else
                            {
                                
                                const basegfx::B2DVector aDiscreteHalfLineVector(getViewInformation2D().getObjectToViewTransformation()
                                    * basegfx::B2DVector(rLineAttribute.getWidth() * 0.5, 0.0));
                                mbHit = checkHairlineHitWithTolerance(
                                    rPolygonCandidate.getB2DPolygon(),
                                    getDiscreteHitTolerance() + aDiscreteHalfLineVector.getLength());
                            }
                        }
                        else
                        {
                            
                            
                            mbHit = checkHairlineHitWithTolerance(rPolygonCandidate.getB2DPolygon(), getDiscreteHitTolerance());
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONWAVEPRIMITIVE2D :
                {
                    if(!getHitTextOnly())
                    {
                        
                        const primitive2d::PolygonWavePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolygonWavePrimitive2D& >(rCandidate));
                        double fLogicHitTolerance(0.0);

                        
                        if(basegfx::fTools::more(rPolygonCandidate.getWaveHeight(), 0.0))
                        {
                            fLogicHitTolerance += rPolygonCandidate.getWaveHeight();
                        }

                        
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
                        
                        const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolyPolygonColorPrimitive2D& >(rCandidate));

                        
                        mbHit = checkFillHitWithTolerance(rPolygonCandidate.getB2DPolyPolygon(), getDiscreteHitTolerance());
                    }

                    break;
                }
                case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D :
                {
                    
                    const primitive2d::TransparencePrimitive2D& rTransCandidate(static_cast< const primitive2d::TransparencePrimitive2D& >(rCandidate));

                    
                    
                    
                    process(rTransCandidate.getChildren());

                    break;
                }
                case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
                {
                    
                    
                    const primitive2d::MaskPrimitive2D& rMaskCandidate(static_cast< const primitive2d::MaskPrimitive2D& >(rCandidate));

                    
                    if(checkFillHitWithTolerance(rMaskCandidate.getMask(), getDiscreteHitTolerance()))
                    {
                        
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
                    
                    break;
                }
                case PRIMITIVE2D_ID_SHADOWPRIMITIVE2D :
                {
                    
                    
                    break;
                }
                case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D :
                case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D :
                {
                    
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
                {
                    if(!getHitTextOnly())
                    {
                        
                        
                        //
                        
                        
                        
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
                    
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));

                    break;
                }
            }
        }

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
