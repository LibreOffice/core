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

#include <drawinglayer/processor3d/cutfindprocessor3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <drawinglayer/primitive3d/hatchtextureprimitive3d.hxx>
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <drawinglayer/primitive3d/hiddengeometryprimitive3d.hxx>



namespace drawinglayer
{
    namespace processor3d
    {
        CutFindProcessor::CutFindProcessor(const geometry::ViewInformation3D& rViewInformation,
            const basegfx::B3DPoint& rFront,
            const basegfx::B3DPoint& rBack,
            bool bAnyHit)
        :   BaseProcessor3D(rViewInformation),
            maFront(rFront),
            maBack(rBack),
            maResult(),
            maCombinedTransform(),
            mbAnyHit(bAnyHit),
            mbUseInvisiblePrimitiveContent(true)
        {
        }

        void CutFindProcessor::processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate)
        {
            if(getAnyHit() && maResult.size())
            {
                
                return;
            }

            
            switch(rCandidate.getPrimitive3DID())
            {
                case PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D :
                {
                    
                    const primitive3d::TransformPrimitive3D& rPrimitive = static_cast< const primitive3d::TransformPrimitive3D& >(rCandidate);

                    
                    const basegfx::B3DPoint aLastFront(maFront);
                    const basegfx::B3DPoint aLastBack(maBack);
                    basegfx::B3DHomMatrix aInverseTrans(rPrimitive.getTransformation());
                    aInverseTrans.invert();
                    maFront *= aInverseTrans;
                    maBack *= aInverseTrans;

                    
                    const geometry::ViewInformation3D aLastViewInformation3D(getViewInformation3D());
                    const geometry::ViewInformation3D aNewViewInformation3D(
                        aLastViewInformation3D.getObjectTransformation() * rPrimitive.getTransformation(),
                        aLastViewInformation3D.getOrientation(),
                        aLastViewInformation3D.getProjection(),
                        aLastViewInformation3D.getDeviceToView(),
                        aLastViewInformation3D.getViewTime(),
                        aLastViewInformation3D.getExtendedInformationSequence());
                    updateViewInformation(aNewViewInformation3D);

                    
                    const basegfx::B3DHomMatrix aLastCombinedTransform(maCombinedTransform);
                    maCombinedTransform = maCombinedTransform * rPrimitive.getTransformation();

                    
                    process(rPrimitive.getChildren());

                    
                    maCombinedTransform = aLastCombinedTransform;
                    updateViewInformation(aLastViewInformation3D);
                    maFront = aLastFront;
                    maBack = aLastBack;
                    break;
                }
                case PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D :
                {
                    
                    
                    
                    break;
                }
                case PRIMITIVE3D_ID_HATCHTEXTUREPRIMITIVE3D :
                {
                    
                    
                    
                    
                    
                    const primitive3d::HatchTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::HatchTexturePrimitive3D& >(rCandidate);
                    process(rPrimitive.getChildren());
                    break;
                }
                case PRIMITIVE3D_ID_HIDDENGEOMETRYPRIMITIVE3D :
                {
                    
                    
                    
                    const primitive3d::HiddenGeometryPrimitive3D& rHiddenGeometry(static_cast< const primitive3d::HiddenGeometryPrimitive3D& >(rCandidate));
                       const primitive3d::Primitive3DSequence& rChildren = rHiddenGeometry.getChildren();

                    if(rChildren.hasElements())
                    {
                        if(getUseInvisiblePrimitiveContent())
                        {
                            process(rChildren);
                        }
                    }

                    break;
                }
                case PRIMITIVE3D_ID_UNIFIEDTRANSPARENCETEXTUREPRIMITIVE3D :
                {
                    const primitive3d::UnifiedTransparenceTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::UnifiedTransparenceTexturePrimitive3D& >(rCandidate);
                       const primitive3d::Primitive3DSequence rChildren = rPrimitive.getChildren();

                    if(rChildren.getLength())
                    {
                        if(1.0 <= rPrimitive.getTransparence())
                        {
                            
                            if(getUseInvisiblePrimitiveContent())
                            {
                                   process(rChildren);
                            }
                        }
                        else if(rPrimitive.getTransparence() >= 0.0 && rPrimitive.getTransparence() < 1.0)
                        {
                            
                            process(rChildren);
                        }
                    }

                    break;
                }
                case PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D :
                {
                    
                    const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const primitive3d::PolyPolygonMaterialPrimitive3D& >(rCandidate);

                    if(!maFront.equal(maBack))
                    {
                           const basegfx::B3DPolyPolygon& rPolyPolygon = rPrimitive.getB3DPolyPolygon();
                        const sal_uInt32 nPolyCount(rPolyPolygon.count());

                        if(nPolyCount)
                        {
                               const basegfx::B3DPolygon aPolygon(rPolyPolygon.getB3DPolygon(0));
                            const sal_uInt32 nPointCount(aPolygon.count());

                            if(nPointCount > 2)
                            {
                                const basegfx::B3DVector aPlaneNormal(aPolygon.getNormal());

                                if(!aPlaneNormal.equalZero())
                                {
                                    const basegfx::B3DPoint aPointOnPlane(aPolygon.getB3DPoint(0));
                                    double fCut(0.0);

                                    if(basegfx::tools::getCutBetweenLineAndPlane(aPlaneNormal, aPointOnPlane, maFront, maBack, fCut))
                                    {
                                        const basegfx::B3DPoint aCutPoint(basegfx::interpolate(maFront, maBack, fCut));

                                        if(basegfx::tools::isInside(rPolyPolygon, aCutPoint, false))
                                        {
                                            
                                            
                                            
                                            maResult.push_back(maCombinedTransform * aCutPoint);
                                        }
                                    }
                                }
                            }
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
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
