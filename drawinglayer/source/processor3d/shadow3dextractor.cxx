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

#include <drawinglayer/processor3d/shadow3dextractor.hxx>
#include <drawinglayer/primitive3d/shadowprimitive3d.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>



using namespace com::sun::star;



namespace drawinglayer
{
    namespace processor3d
    {
        
        
        void Shadow3DExtractingProcessor::processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate)
        {
            
            switch(rCandidate.getPrimitive3DID())
            {
                case PRIMITIVE3D_ID_SHADOWPRIMITIVE3D :
                {
                    
                    const primitive3d::ShadowPrimitive3D& rPrimitive = static_cast< const primitive3d::ShadowPrimitive3D& >(rCandidate);

                    
                    primitive2d::Primitive2DVector aNewSubList;
                    primitive2d::Primitive2DVector* pLastTargetSequence = mpPrimitive2DSequence;
                    mpPrimitive2DSequence = &aNewSubList;

                    
                    const bool bLastConvert(mbConvert);
                    mbConvert = true;

                    
                    const bool bLastUseProjection(mbUseProjection);
                    mbUseProjection = rPrimitive.getShadow3D();

                    
                    process(rPrimitive.getChildren());

                    
                    mbUseProjection = bLastUseProjection;
                    mbConvert = bLastConvert;
                    mpPrimitive2DSequence = pLastTargetSequence;

                    
                    
                    primitive2d::BasePrimitive2D* pNew = new primitive2d::ShadowPrimitive2D(
                        rPrimitive.getShadowTransform(),
                        rPrimitive.getShadowColor(),
                        primitive2d::Primitive2DVectorToPrimitive2DSequence(aNewSubList));

                    if(basegfx::fTools::more(rPrimitive.getShadowTransparence(), 0.0))
                    {
                        
                        const primitive2d::Primitive2DReference xRef(pNew);
                        const primitive2d::Primitive2DSequence aNewTransPrimitiveVector(&xRef, 1);

                        pNew = new primitive2d::UnifiedTransparencePrimitive2D(
                            aNewTransPrimitiveVector,
                            rPrimitive.getShadowTransparence());
                    }

                    mpPrimitive2DSequence->push_back(pNew);

                    break;
                }
                case PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D :
                {
                    
                    const primitive3d::TransformPrimitive3D& rPrimitive = static_cast< const primitive3d::TransformPrimitive3D& >(rCandidate);
                    const geometry::ViewInformation3D aLastViewInformation3D(getViewInformation3D());

                    
                    const geometry::ViewInformation3D aNewViewInformation3D(
                        aLastViewInformation3D.getObjectTransformation() * rPrimitive.getTransformation(),
                        aLastViewInformation3D.getOrientation(),
                        aLastViewInformation3D.getProjection(),
                        aLastViewInformation3D.getDeviceToView(),
                        aLastViewInformation3D.getViewTime(),
                        aLastViewInformation3D.getExtendedInformationSequence());
                    updateViewInformation(aNewViewInformation3D);

                    if(mbShadowProjectionIsValid)
                    {
                        
                        maWorldToEye = getViewInformation3D().getOrientation() * getViewInformation3D().getObjectTransformation();
                        maEyeToView = getViewInformation3D().getDeviceToView() * getViewInformation3D().getProjection();
                    }

                    
                    process(rPrimitive.getChildren());

                    
                    updateViewInformation(aLastViewInformation3D);

                    if(mbShadowProjectionIsValid)
                    {
                        
                        maWorldToEye = getViewInformation3D().getOrientation() * getViewInformation3D().getObjectTransformation();
                        maEyeToView = getViewInformation3D().getDeviceToView() * getViewInformation3D().getProjection();
                    }
                    break;
                }
                case PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D :
                {
                    
                    if(mbConvert)
                    {
                        const primitive3d::PolygonHairlinePrimitive3D& rPrimitive = static_cast< const primitive3d::PolygonHairlinePrimitive3D& >(rCandidate);
                        basegfx::B2DPolygon a2DHairline;

                        if(mbUseProjection)
                        {
                            if(mbShadowProjectionIsValid)
                            {
                                a2DHairline = impDoShadowProjection(rPrimitive.getB3DPolygon());
                            }
                        }
                        else
                        {
                            a2DHairline = basegfx::tools::createB2DPolygonFromB3DPolygon(rPrimitive.getB3DPolygon(), getViewInformation3D().getObjectToView());
                        }

                        if(a2DHairline.count())
                        {
                            a2DHairline.transform(getObjectTransformation());
                            mpPrimitive2DSequence->push_back(
                                new primitive2d::PolygonHairlinePrimitive2D(
                                    a2DHairline,
                                    maPrimitiveColor));
                        }
                    }
                    break;
                }
                case PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D :
                {
                    
                    if(mbConvert)
                    {
                        const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const primitive3d::PolyPolygonMaterialPrimitive3D& >(rCandidate);
                        basegfx::B2DPolyPolygon a2DFill;

                        if(mbUseProjection)
                        {
                            if(mbShadowProjectionIsValid)
                            {
                                a2DFill = impDoShadowProjection(rPrimitive.getB3DPolyPolygon());
                            }
                        }
                        else
                        {
                            a2DFill = basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(rPrimitive.getB3DPolyPolygon(), getViewInformation3D().getObjectToView());
                        }

                        if(a2DFill.count())
                        {
                            a2DFill.transform(getObjectTransformation());
                            mpPrimitive2DSequence->push_back(
                                new primitive2d::PolyPolygonColorPrimitive2D(
                                    a2DFill,
                                    maPrimitiveColor));
                        }
                    }
                    break;
                }
                default :
                {
                    
                    process(rCandidate.get3DDecomposition(getViewInformation3D()));
                    break;
                }
            }
        }

        Shadow3DExtractingProcessor::Shadow3DExtractingProcessor(
            const geometry::ViewInformation3D& rViewInformation,
            const basegfx::B2DHomMatrix& rObjectTransformation,
            const basegfx::B3DVector& rLightNormal,
            double fShadowSlant,
            const basegfx::B3DRange& rContained3DRange)
        :   BaseProcessor3D(rViewInformation),
            maPrimitive2DSequence(),
            mpPrimitive2DSequence(&maPrimitive2DSequence),
            maObjectTransformation(rObjectTransformation),
            maWorldToEye(),
            maEyeToView(),
            maLightNormal(rLightNormal),
            maShadowPlaneNormal(),
            maPlanePoint(),
            mfLightPlaneScalar(0.0),
            maPrimitiveColor(),
            mbShadowProjectionIsValid(false),
            mbConvert(false),
            mbUseProjection(false)
        {
            
            maLightNormal.normalize();
            maShadowPlaneNormal = basegfx::B3DVector(0.0, sin(fShadowSlant), cos(fShadowSlant));
            maShadowPlaneNormal.normalize();
            mfLightPlaneScalar = maLightNormal.scalar(maShadowPlaneNormal);

            
            if(basegfx::fTools::more(mfLightPlaneScalar, 0.0))
            {
                
                maWorldToEye = getViewInformation3D().getOrientation() * getViewInformation3D().getObjectTransformation();
                maEyeToView = getViewInformation3D().getDeviceToView() * getViewInformation3D().getProjection();

                
                basegfx::B3DRange aContained3DRange(rContained3DRange);
                aContained3DRange.transform(getWorldToEye());
                maPlanePoint.setX(maShadowPlaneNormal.getX() < 0.0 ? aContained3DRange.getMinX() : aContained3DRange.getMaxX());
                maPlanePoint.setY(maShadowPlaneNormal.getY() > 0.0 ? aContained3DRange.getMinY() : aContained3DRange.getMaxY());
                maPlanePoint.setZ(aContained3DRange.getMinZ() - (aContained3DRange.getDepth() / 8.0));

                
                mbShadowProjectionIsValid = true;
            }
        }

        Shadow3DExtractingProcessor::~Shadow3DExtractingProcessor()
        {
            OSL_ENSURE(0 == maPrimitive2DSequence.size(),
                "OOps, someone used Shadow3DExtractingProcessor, but did not fetch the results (!)");
            for(sal_uInt32 a(0); a < maPrimitive2DSequence.size(); a++)
            {
                delete maPrimitive2DSequence[a];
            }
        }

        basegfx::B2DPolygon Shadow3DExtractingProcessor::impDoShadowProjection(const basegfx::B3DPolygon& rSource)
        {
            basegfx::B2DPolygon aRetval;

            for(sal_uInt32 a(0L); a < rSource.count(); a++)
            {
                
                basegfx::B3DPoint aCandidate(rSource.getB3DPoint(a));
                aCandidate *= getWorldToEye();

                
                
                
                
                
                const double fCut(basegfx::B3DVector(maPlanePoint - aCandidate).scalar(maShadowPlaneNormal) / mfLightPlaneScalar);
                aCandidate += maLightNormal * fCut;

                
                aCandidate *= getEyeToView();
                aRetval.append(basegfx::B2DPoint(aCandidate.getX(), aCandidate.getY()));
            }

            
            aRetval.setClosed(rSource.isClosed());

            return aRetval;
        }

        basegfx::B2DPolyPolygon Shadow3DExtractingProcessor::impDoShadowProjection(const basegfx::B3DPolyPolygon& rSource)
        {
            basegfx::B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < rSource.count(); a++)
            {
                aRetval.append(impDoShadowProjection(rSource.getB3DPolygon(a)));
            }

            return aRetval;
        }

        const primitive2d::Primitive2DSequence Shadow3DExtractingProcessor::getPrimitive2DSequence() const
        {
            return Primitive2DVectorToPrimitive2DSequence(maPrimitive2DSequence);
        }

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
