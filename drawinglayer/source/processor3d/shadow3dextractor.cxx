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

#include <processor3d/shadow3dextractor.hxx>
#include <primitive3d/shadowprimitive3d.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <rtl/ref.hxx>


using namespace com::sun::star;


namespace drawinglayer::processor3d
{
        // as tooling, the process() implementation takes over API handling and calls this
        // virtual render method when the primitive implementation is BasePrimitive3D-based.
        void Shadow3DExtractingProcessor::processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate)
        {
            // it is a BasePrimitive3D implementation, use getPrimitive3DID() call for switch
            switch(rCandidate.getPrimitive3DID())
            {
                case PRIMITIVE3D_ID_SHADOWPRIMITIVE3D :
                {
                    // shadow3d object. Call recursive with content and start conversion
                    const primitive3d::ShadowPrimitive3D& rPrimitive = static_cast< const primitive3d::ShadowPrimitive3D& >(rCandidate);

                    // set new target
                    primitive2d::Primitive2DContainer aNewSubList;
                    primitive2d::Primitive2DContainer* pLastTargetSequence = mpPrimitive2DSequence;
                    mpPrimitive2DSequence = &aNewSubList;

                    // activate convert
                    const bool bLastConvert(mbConvert);
                    mbConvert = true;

                    // set projection flag
                    const bool bLastUseProjection(mbUseProjection);
                    mbUseProjection = rPrimitive.getShadow3D();

                    // process content
                    process(rPrimitive.getChildren());

                    // restore values
                    mbUseProjection = bLastUseProjection;
                    mbConvert = bLastConvert;
                    mpPrimitive2DSequence = pLastTargetSequence;

                    // create 2d shadow primitive with result. This also fetches all entries
                    // from aNewSubList, so there is no need to delete them
                    rtl::Reference<primitive2d::BasePrimitive2D> pNew = new primitive2d::ShadowPrimitive2D(
                        rPrimitive.getShadowTransform(),
                        rPrimitive.getShadowColor(),
                        0,  // shadow3d doesn't have rPrimitive.getShadowBlur() yet.
                        std::move(aNewSubList));

                    if(basegfx::fTools::more(rPrimitive.getShadowTransparence(), 0.0))
                    {
                        // create simpleTransparencePrimitive, add created primitives
                        primitive2d::Primitive2DContainer aNewTransPrimitiveVector { pNew };

                        pNew = new primitive2d::UnifiedTransparencePrimitive2D(
                            std::move(aNewTransPrimitiveVector),
                            rPrimitive.getShadowTransparence());
                    }

                    mpPrimitive2DSequence->push_back(pNew);

                    break;
                }
                case PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D :
                {
                    // transform group. Remember current transformations
                    const primitive3d::TransformPrimitive3D& rPrimitive = static_cast< const primitive3d::TransformPrimitive3D& >(rCandidate);
                    const geometry::ViewInformation3D aLastViewInformation3D(getViewInformation3D());

                    // create new transformation; add new object transform from right side
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
                        // update buffered WorldToEye and EyeToView
                        maWorldToEye = getViewInformation3D().getOrientation() * getViewInformation3D().getObjectTransformation();
                        maEyeToView = getViewInformation3D().getDeviceToView() * getViewInformation3D().getProjection();
                    }

                    // let break down
                    process(rPrimitive.getChildren());

                    // restore transformations
                    updateViewInformation(aLastViewInformation3D);

                    if(mbShadowProjectionIsValid)
                    {
                        // update buffered WorldToEye and EyeToView
                        maWorldToEye = getViewInformation3D().getOrientation() * getViewInformation3D().getObjectTransformation();
                        maEyeToView = getViewInformation3D().getDeviceToView() * getViewInformation3D().getProjection();
                    }
                    break;
                }
                case PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D :
                {
                    // PolygonHairlinePrimitive3D
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
                            a2DHairline = basegfx::utils::createB2DPolygonFromB3DPolygon(rPrimitive.getB3DPolygon(), getViewInformation3D().getObjectToView());
                        }

                        if(a2DHairline.count())
                        {
                            a2DHairline.transform(getObjectTransformation());
                            mpPrimitive2DSequence->push_back(
                                new primitive2d::PolygonHairlinePrimitive2D(
                                    a2DHairline,
                                    basegfx::BColor()));
                        }
                    }
                    break;
                }
                case PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D :
                {
                    // PolyPolygonMaterialPrimitive3D
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
                            a2DFill = basegfx::utils::createB2DPolyPolygonFromB3DPolyPolygon(rPrimitive.getB3DPolyPolygon(), getViewInformation3D().getObjectToView());
                        }

                        if(a2DFill.count())
                        {
                            a2DFill.transform(getObjectTransformation());
                            mpPrimitive2DSequence->push_back(
                                new primitive2d::PolyPolygonColorPrimitive2D(
                                    a2DFill,
                                    basegfx::BColor()));
                        }
                    }
                    break;
                }
                default :
                {
                    // process recursively
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
            mpPrimitive2DSequence(&maPrimitive2DSequence),
            maObjectTransformation(rObjectTransformation),
            maLightNormal(rLightNormal),
            mfLightPlaneScalar(0.0),
            mbShadowProjectionIsValid(false),
            mbConvert(false),
            mbUseProjection(false)
        {
            // normalize light normal, get and normalize shadow plane normal and calculate scalar from it
            maLightNormal.normalize();
            maShadowPlaneNormal = basegfx::B3DVector(0.0, sin(fShadowSlant), cos(fShadowSlant));
            maShadowPlaneNormal.normalize();
            mfLightPlaneScalar = maLightNormal.scalar(maShadowPlaneNormal);

            // use only when scalar is > 0.0, so the light is in front of the object
            if(!basegfx::fTools::more(mfLightPlaneScalar, 0.0))
                return;

            // prepare buffered WorldToEye and EyeToView
            maWorldToEye = getViewInformation3D().getOrientation() * getViewInformation3D().getObjectTransformation();
            maEyeToView = getViewInformation3D().getDeviceToView() * getViewInformation3D().getProjection();

            // calculate range to get front edge around which to rotate the shadow's projection
            basegfx::B3DRange aContained3DRange(rContained3DRange);
            aContained3DRange.transform(getWorldToEye());
            maPlanePoint.setX(maShadowPlaneNormal.getX() < 0.0 ? aContained3DRange.getMinX() : aContained3DRange.getMaxX());
            maPlanePoint.setY(maShadowPlaneNormal.getY() > 0.0 ? aContained3DRange.getMinY() : aContained3DRange.getMaxY());
            maPlanePoint.setZ(aContained3DRange.getMinZ() - (aContained3DRange.getDepth() / 8.0));

            // set flag that shadow projection is prepared and allowed
            mbShadowProjectionIsValid = true;
        }

        Shadow3DExtractingProcessor::~Shadow3DExtractingProcessor()
        {
            OSL_ENSURE(maPrimitive2DSequence.empty(),
                "OOps, someone used Shadow3DExtractingProcessor, but did not fetch the results (!)");
        }

        basegfx::B2DPolygon Shadow3DExtractingProcessor::impDoShadowProjection(const basegfx::B3DPolygon& rSource)
        {
            basegfx::B2DPolygon aRetval;

            for(sal_uInt32 a(0); a < rSource.count(); a++)
            {
                // get point, transform to eye coordinate system
                basegfx::B3DPoint aCandidate(rSource.getB3DPoint(a));
                aCandidate *= getWorldToEye();

                // we are in eye coordinates
                // ray is (aCandidate + fCut * maLightNormal)
                // plane is (maPlanePoint, maShadowPlaneNormal)
                // maLightNormal.scalar(maShadowPlaneNormal) is already in mfLightPlaneScalar and > 0.0
                // get cut point of ray with shadow plane
                const double fCut(basegfx::B3DVector(maPlanePoint - aCandidate).scalar(maShadowPlaneNormal) / mfLightPlaneScalar);
                aCandidate += maLightNormal * fCut;

                // transform to view, use 2d coordinates
                aCandidate *= maEyeToView;
                aRetval.append(basegfx::B2DPoint(aCandidate.getX(), aCandidate.getY()));
            }

            // copy closed flag
            aRetval.setClosed(rSource.isClosed());

            return aRetval;
        }

        basegfx::B2DPolyPolygon Shadow3DExtractingProcessor::impDoShadowProjection(const basegfx::B3DPolyPolygon& rSource)
        {
            basegfx::B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0); a < rSource.count(); a++)
            {
                aRetval.append(impDoShadowProjection(rSource.getB3DPolygon(a)));
            }

            return aRetval;
        }

        const primitive2d::Primitive2DContainer& Shadow3DExtractingProcessor::getPrimitive2DSequence() const
        {
            return maPrimitive2DSequence;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
