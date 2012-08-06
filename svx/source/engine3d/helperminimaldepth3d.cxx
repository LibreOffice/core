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


#include <helperminimaldepth3d.hxx>
#include <drawinglayer/processor3d/baseprocessor3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#include <svx/sdr/contact/viewcontactofe3d.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <svx/obj3d.hxx>
#include <svx/scene3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        class MinimalDephInViewExtractor : public BaseProcessor3D
        {
        private:
            // the value which will be fetched as result
            double                                  mfMinimalDepth;

            // as tooling, the process() implementation takes over API handling and calls this
            // virtual render method when the primitive implementation is BasePrimitive3D-based.
            virtual void processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate);

        public:
            MinimalDephInViewExtractor(const geometry::ViewInformation3D& rViewInformation)
            :   BaseProcessor3D(rViewInformation),
                mfMinimalDepth(DBL_MAX)
            {}

            // data access
            double getMinimalDepth() const { return mfMinimalDepth; }
        };

        void MinimalDephInViewExtractor::processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate)
        {
            // it is a BasePrimitive3D implementation, use getPrimitive3DID() call for switch
            switch(rCandidate.getPrimitive3DID())
            {
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

                    // let break down
                    process(rPrimitive.getChildren());

                    // restore transformations
                    updateViewInformation(aLastViewInformation3D);
                    break;
                }
                case PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D :
                {
                    // PolygonHairlinePrimitive3D
                    const primitive3d::PolygonHairlinePrimitive3D& rPrimitive = static_cast< const primitive3d::PolygonHairlinePrimitive3D& >(rCandidate);
                       const basegfx::B3DPolygon& rPolygon = rPrimitive.getB3DPolygon();
                    const sal_uInt32 nCount(rPolygon.count());

                    for(sal_uInt32 a(0); a < nCount; a++)
                    {
                        const basegfx::B3DPoint aPointInView(getViewInformation3D().getObjectToView() * rPolygon.getB3DPoint(a));

                        if(aPointInView.getZ() < mfMinimalDepth)
                        {
                            mfMinimalDepth = aPointInView.getZ();
                        }
                    }

                    break;
                }
                case PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D :
                {
                    // PolyPolygonMaterialPrimitive3D
                    const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const primitive3d::PolyPolygonMaterialPrimitive3D& >(rCandidate);
                       const basegfx::B3DPolyPolygon& rPolyPolygon = rPrimitive.getB3DPolyPolygon();
                    const sal_uInt32 nPolyCount(rPolyPolygon.count());

                    for(sal_uInt32 a(0); a < nPolyCount; a++)
                    {
                           const basegfx::B3DPolygon aPolygon(rPolyPolygon.getB3DPolygon(a));
                        const sal_uInt32 nCount(aPolygon.count());

                        for(sal_uInt32 b(0); b < nCount; b++)
                        {
                            const basegfx::B3DPoint aPointInView(getViewInformation3D().getObjectToView() * aPolygon.getB3DPoint(b));

                            if(aPointInView.getZ() < mfMinimalDepth)
                            {
                                mfMinimalDepth = aPointInView.getZ();
                            }
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
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// changed to create values using VCs, Primitive3DSequence and ViewInformation3D to allow
// removal of old 3D bucket geometry. There is one slight difference in the result, it's
// in [0.0 .. 1.0] for Z-Depth since the scaling of the scene as 2D object is no longer
// part of the 3D transformations. This could be added since the ViewContactOfE3dScene is
// given, but is not needed since the permutation of the depth values needs only be correct
// relative to each other

double getMinimalDepthInViewCoordinates(const E3dCompoundObject& rObject)
{
    // this is a E3dCompoundObject, so it cannot be a scene (which is a E3dObject).
    // Get primitive sequence using VC
    const sdr::contact::ViewContactOfE3d& rVCObject = static_cast< sdr::contact::ViewContactOfE3d& >(rObject.GetViewContact());
    const drawinglayer::primitive3d::Primitive3DSequence aPrimitives = rVCObject.getViewIndependentPrimitive3DSequence();
    double fRetval(DBL_MAX);

    if(aPrimitives.hasElements())
    {
        const E3dScene* pScene = rObject.GetScene();

        if(pScene)
        {
            // get ViewInformation3D from scene using VC
            const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pScene->GetViewContact());
            const drawinglayer::geometry::ViewInformation3D aViewInfo3D(rVCScene.getViewInformation3D());

            // the scene's object transformation is already part of aViewInfo3D.getObjectTransformation()
            // for historical reasons (see ViewContactOfE3dScene::createViewInformation3D for more info)
            // and the object's transform is part of aPrimitives (and taken into account when decomposing
            // to PolygonHairlinePrimitive3D and PolyPolygonMaterialPrimitive3D). The missing part may be
            // some Scene SdrObjects lying in-between which may need to be added. This is e.g. used in chart,
            // and generally allowed in 3d scenes an their 3d object hierarchy
            basegfx::B3DHomMatrix aInBetweenSceneMatrix;
            E3dScene* pParentScene = dynamic_cast< E3dScene* >(rObject.GetParentObj());

            while(pParentScene && pParentScene != pScene)
            {
                aInBetweenSceneMatrix = pParentScene->GetTransform() * aInBetweenSceneMatrix;
                pParentScene = dynamic_cast< E3dScene* >(pParentScene->GetParentObj());
            }

            // build new ViewInformation containing all transforms
            const drawinglayer::geometry::ViewInformation3D aNewViewInformation3D(
                aViewInfo3D.getObjectTransformation() * aInBetweenSceneMatrix,
                aViewInfo3D.getOrientation(),
                aViewInfo3D.getProjection(),
                aViewInfo3D.getDeviceToView(),
                aViewInfo3D.getViewTime(),
                aViewInfo3D.getExtendedInformationSequence());

            // create extractor helper, proccess geometry and get return value
            drawinglayer::processor3d::MinimalDephInViewExtractor aExtractor(aNewViewInformation3D);
            aExtractor.process(aPrimitives);
            fRetval = aExtractor.getMinimalDepth();
        }
    }

    return fRetval;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
