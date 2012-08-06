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

#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <svx/polysc3d.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/contact/viewobjectcontactofe3dscene.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b3drange.hxx>
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <svx/sdr/contact/viewcontactofe3d.hxx>
#include <drawinglayer/primitive2d/sceneprimitive2d.hxx>
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace
{
    // pActiveVC is only true if ghosted is still activated and maybe needs to be switched off in this path
    void createSubPrimitive3DVector(
        const sdr::contact::ViewContact& rCandidate,
        drawinglayer::primitive3d::Primitive3DSequence& o_rAllTarget,
        drawinglayer::primitive3d::Primitive3DSequence* o_pVisibleTarget,
        const SetOfByte* pVisibleLayerSet,
        const bool bTestSelectedVisibility)
    {
        const sdr::contact::ViewContactOfE3dScene* pViewContactOfE3dScene = dynamic_cast< const sdr::contact::ViewContactOfE3dScene* >(&rCandidate);

        if(pViewContactOfE3dScene)
        {
            const sal_uInt32 nChildrenCount(rCandidate.GetObjectCount());

            if(nChildrenCount)
            {
                // provide new collection sequences
                drawinglayer::primitive3d::Primitive3DSequence aNewAllTarget;
                drawinglayer::primitive3d::Primitive3DSequence aNewVisibleTarget;

                // add children recursively
                for(sal_uInt32 a(0L); a < nChildrenCount; a++)
                {
                    createSubPrimitive3DVector(
                        rCandidate.GetViewContact(a),
                        aNewAllTarget,
                        o_pVisibleTarget ? &aNewVisibleTarget : 0,
                        pVisibleLayerSet,
                        bTestSelectedVisibility);
                }

                // create transform primitive for the created content combining content and transformtion
                const drawinglayer::primitive3d::Primitive3DReference xReference(new drawinglayer::primitive3d::TransformPrimitive3D(
                    pViewContactOfE3dScene->GetE3dScene().GetTransform(),
                    aNewAllTarget));

                // add created content to all target
                drawinglayer::primitive3d::appendPrimitive3DReferenceToPrimitive3DSequence(o_rAllTarget, xReference);

                // add created content to visibiel target if exists
                if(o_pVisibleTarget)
                {
                    drawinglayer::primitive3d::appendPrimitive3DReferenceToPrimitive3DSequence(*o_pVisibleTarget, xReference);
                }
            }
        }
        else
        {
            // access view independent representation of rCandidate
            const sdr::contact::ViewContactOfE3d* pViewContactOfE3d = dynamic_cast< const sdr::contact::ViewContactOfE3d* >(&rCandidate);

            if(pViewContactOfE3d)
            {
                drawinglayer::primitive3d::Primitive3DSequence xPrimitive3DSeq(pViewContactOfE3d->getViewIndependentPrimitive3DSequence());

                if(xPrimitive3DSeq.hasElements())
                {
                    // add to all target vector
                    drawinglayer::primitive3d::appendPrimitive3DSequenceToPrimitive3DSequence(o_rAllTarget, xPrimitive3DSeq);

                    if(o_pVisibleTarget)
                    {
                        // test visibility. Primitive is visible when both tests are true (AND)
                        bool bVisible(true);

                        if(pVisibleLayerSet)
                        {
                            // test layer visibility
                            const E3dObject& rE3dObject = pViewContactOfE3d->GetE3dObject();
                            const SdrLayerID aLayerID(rE3dObject.GetLayer());

                            bVisible = pVisibleLayerSet->IsSet(aLayerID);
                        }

                        if(bVisible && bTestSelectedVisibility)
                        {
                            // test selected visibility (see 3D View's DrawMarkedObj implementation)
                            const E3dObject& rE3dObject = pViewContactOfE3d->GetE3dObject();

                            bVisible = rE3dObject.GetSelected();
                        }

                        if(bVisible && o_pVisibleTarget)
                        {
                            // add to visible target vector
                            drawinglayer::primitive3d::appendPrimitive3DSequenceToPrimitive3DSequence(*o_pVisibleTarget, xPrimitive3DSeq);
                        }
                    }
                }
            }
        }
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // Create a Object-Specific ViewObjectContact, set ViewContact and
        // ObjectContact. Always needs to return something.
        ViewObjectContact& ViewContactOfE3dScene::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfE3dScene(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContactOfE3dScene::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        ViewContactOfE3dScene::ViewContactOfE3dScene(E3dScene& rScene)
        :   ViewContactOfSdrObj(rScene),
            maViewInformation3D(),
            maObjectTransformation(),
            maSdrSceneAttribute(),
            maSdrLightingAttribute()
        {
        }

        void ViewContactOfE3dScene::createViewInformation3D(const basegfx::B3DRange& rContentRange)
        {
            basegfx::B3DHomMatrix aTransformation;
            basegfx::B3DHomMatrix aOrientation;
            basegfx::B3DHomMatrix aProjection;
            basegfx::B3DHomMatrix aDeviceToView;

            // create transformation (scene as group's transformation)
            // For historical reasons, the outmost scene's transformation is handles as part of the
            // view transformation. This means that the BoundRect of the contained 3D Objects is
            // without that transformation and makes it necessary to NOT add the first scene to the
            // Primitive3DSequence of contained objects.
            {
                aTransformation = GetE3dScene().GetTransform();
            }

            // create orientation (world to camera coordinate system)
            {
                // calculate orientation from VRP, VPN and VUV
                const B3dCamera& rSceneCamera = GetE3dScene().GetCameraSet();
                const basegfx::B3DPoint aVRP(rSceneCamera.GetVRP());
                const basegfx::B3DVector aVPN(rSceneCamera.GetVRP());
                const basegfx::B3DVector aVUV(rSceneCamera.GetVUV());

                aOrientation.orientation(aVRP, aVPN, aVUV);
            }

            // create projection (camera coordinate system to relative 2d where X,Y and Z are [0.0 .. 1.0])
            {
                const basegfx::B3DHomMatrix aWorldToCamera(aOrientation * aTransformation);
                basegfx::B3DRange aCameraRange(rContentRange);
                aCameraRange.transform(aWorldToCamera);

                // remember Z-Values, but change orientation
                const double fMinZ(-aCameraRange.getMaxZ());
                const double fMaxZ(-aCameraRange.getMinZ());

                // construct temorary matrix from world to device. Use unit values here to measure expansion
                basegfx::B3DHomMatrix aWorldToDevice(aWorldToCamera);
                const drawinglayer::attribute::SdrSceneAttribute& rSdrSceneAttribute = getSdrSceneAttribute();

                if(::com::sun::star::drawing::ProjectionMode_PERSPECTIVE == rSdrSceneAttribute.getProjectionMode())
                {
                    aWorldToDevice.frustum(-1.0, 1.0, -1.0, 1.0, fMinZ, fMaxZ);
                }
                else
                {
                    aWorldToDevice.ortho(-1.0, 1.0, -1.0, 1.0, fMinZ, fMaxZ);
                }

                // create B3DRange in device. This will create the real used ranges
                // in camera space. Do not use the Z-Values, though.
                basegfx::B3DRange aDeviceRange(rContentRange);
                aDeviceRange.transform(aWorldToDevice);

                // set projection
                if(::com::sun::star::drawing::ProjectionMode_PERSPECTIVE == rSdrSceneAttribute.getProjectionMode())
                {
                    aProjection.frustum(
                        aDeviceRange.getMinX(), aDeviceRange.getMaxX(),
                        aDeviceRange.getMinY(), aDeviceRange.getMaxY(),
                        fMinZ, fMaxZ);
                }
                else
                {
                    aProjection.ortho(
                        aDeviceRange.getMinX(), aDeviceRange.getMaxX(),
                        aDeviceRange.getMinY(), aDeviceRange.getMaxY(),
                        fMinZ, fMaxZ);
                }
            }

            // create device to view transform
            {
                // create standard deviceToView projection for geometry
                // input is [-1.0 .. 1.0] in X,Y and Z. bring to [0.0 .. 1.0]. Also
                // necessary to flip Y due to screen orientation
                // Z is not needed, but will also be brought to [0.0 .. 1.0]
                aDeviceToView.scale(0.5, -0.5, 0.5);
                aDeviceToView.translate(0.5, 0.5, 0.5);
            }

            const uno::Sequence< beans::PropertyValue > aEmptyProperties;
            maViewInformation3D = drawinglayer::geometry::ViewInformation3D(
                aTransformation, aOrientation, aProjection,
                aDeviceToView, 0.0, aEmptyProperties);
        }

        void ViewContactOfE3dScene::createObjectTransformation()
        {
            // create 2d Object Transformation from relative point in 2d scene to world
            const Rectangle& rRectangle = GetE3dScene().GetSnapRect();

            maObjectTransformation.set(0, 0, rRectangle.getWidth());
            maObjectTransformation.set(1, 1, rRectangle.getHeight());
            maObjectTransformation.set(0, 2, rRectangle.Left());
            maObjectTransformation.set(1, 2, rRectangle.Top());
        }

        void ViewContactOfE3dScene::createSdrSceneAttribute()
        {
            const SfxItemSet& rItemSet = GetE3dScene().GetMergedItemSet();
            maSdrSceneAttribute = drawinglayer::primitive2d::createNewSdrSceneAttribute(rItemSet);
        }

        void ViewContactOfE3dScene::createSdrLightingAttribute()
        {
            const SfxItemSet& rItemSet = GetE3dScene().GetMergedItemSet();
            maSdrLightingAttribute = drawinglayer::primitive2d::createNewSdrLightingAttribute(rItemSet);
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfE3dScene::createScenePrimitive2DSequence(
            const SetOfByte* pLayerVisibility) const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const sal_uInt32 nChildrenCount(GetObjectCount());

            if(nChildrenCount)
            {
                // create 3d scene primitive with visible content tested against rLayerVisibility
                drawinglayer::primitive3d::Primitive3DSequence aAllSequence;
                drawinglayer::primitive3d::Primitive3DSequence aVisibleSequence;
                const bool bTestLayerVisibility(0 != pLayerVisibility);
                const bool bTestSelectedVisibility(GetE3dScene().GetDrawOnlySelected());
                const bool bTestVisibility(bTestLayerVisibility || bTestSelectedVisibility);

                // add children recursively. Do NOT start with (*this), this would create
                // a 3D transformPrimitive for the start scene. While this is theoretically not
                // a bad thing, for historical reasons the transformation of the outmost scene
                // is seen as part of the ViewTransformation (see text in createViewInformation3D)
                for(sal_uInt32 a(0L); a < nChildrenCount; a++)
                {
                    createSubPrimitive3DVector(
                        GetViewContact(a),
                        aAllSequence,
                        bTestLayerVisibility ? &aVisibleSequence : 0,
                        bTestLayerVisibility ? pLayerVisibility : 0,
                        bTestSelectedVisibility);
                }

                const sal_uInt32 nAllSize(aAllSequence.hasElements() ? aAllSequence.getLength() : 0);
                const sal_uInt32 nVisibleSize(aVisibleSequence.hasElements() ? aVisibleSequence.getLength() : 0);

                if((bTestVisibility && nVisibleSize) || nAllSize)
                {
                    // for getting the 3D range using getB3DRangeFromPrimitive3DSequence a ViewInformation3D
                    // needs to be given for evtl. decompositions. At the same time createViewInformation3D
                    // currently is based on creating the target-ViewInformation3D using a given range. To
                    // get the true range, use a neutral ViewInformation3D here. This leaves all matrices
                    // on identity and the time on 0.0.
                    const uno::Sequence< beans::PropertyValue > aEmptyProperties;
                    const drawinglayer::geometry::ViewInformation3D aNeutralViewInformation3D(aEmptyProperties);
                    const basegfx::B3DRange aContentRange(
                        drawinglayer::primitive3d::getB3DRangeFromPrimitive3DSequence(aAllSequence, aNeutralViewInformation3D));

                    // create 2d primitive 3dscene with generated sub-list from collector
                    const drawinglayer::primitive2d::Primitive2DReference xReference(
                        new drawinglayer::primitive2d::ScenePrimitive2D(
                            bTestVisibility ? aVisibleSequence : aAllSequence,
                            getSdrSceneAttribute(),
                            getSdrLightingAttribute(),
                            getObjectTransformation(),
                            getViewInformation3D(aContentRange)));

                    xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                }
            }

            // always append an invisible outline for the cases where no visible content exists
            drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(xRetval,
                drawinglayer::primitive2d::createHiddenGeometryPrimitives2D(
                    false, getObjectTransformation()));

            return xRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfE3dScene::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            if(GetObjectCount())
            {
                // create a default ScenePrimitive2D (without visibility test of members)
                xRetval = createScenePrimitive2DSequence(0);
            }

            return xRetval;
        }

        void ViewContactOfE3dScene::ActionChanged()
        {
            // call parent
            ViewContactOfSdrObj::ActionChanged();

            // mark locally cached values as invalid
            maViewInformation3D = drawinglayer::geometry::ViewInformation3D();
            maObjectTransformation.identity();
            maSdrSceneAttribute = drawinglayer::attribute::SdrSceneAttribute();
            maSdrLightingAttribute = drawinglayer::attribute::SdrLightingAttribute();
        }

        const drawinglayer::geometry::ViewInformation3D& ViewContactOfE3dScene::getViewInformation3D() const
        {
            if(maViewInformation3D.isDefault())
            {
                // this version will create the content range on demand locally and thus is less
                // performant than the other one. Since the information is buffered the planned
                // behaviour is that the version with the given range is used initially.
                basegfx::B3DRange aContentRange(getAllContentRange3D());

                if(aContentRange.isEmpty())
                {
                    // empty scene, no 3d action should be necessary. Prepare some
                    // fallback size
                    OSL_FAIL("No need to get ViewInformation3D from an empty scene (!)");
                    aContentRange.expand(basegfx::B3DPoint(-100.0, -100.0, -100.0));
                    aContentRange.expand(basegfx::B3DPoint( 100.0,  100.0,  100.0));
                }

                const_cast < ViewContactOfE3dScene* >(this)->createViewInformation3D(aContentRange);
            }

            return maViewInformation3D;
        }

        const drawinglayer::geometry::ViewInformation3D& ViewContactOfE3dScene::getViewInformation3D(const basegfx::B3DRange& rContentRange) const
        {
            if(maViewInformation3D.isDefault())
            {
                const_cast < ViewContactOfE3dScene* >(this)->createViewInformation3D(rContentRange);
            }

            return maViewInformation3D;
        }

        const basegfx::B2DHomMatrix& ViewContactOfE3dScene::getObjectTransformation() const
        {
            if(maObjectTransformation.isIdentity())
            {
                const_cast < ViewContactOfE3dScene* >(this)->createObjectTransformation();
            }

            return maObjectTransformation;
        }

        const drawinglayer::attribute::SdrSceneAttribute& ViewContactOfE3dScene::getSdrSceneAttribute() const
        {
            if(maSdrSceneAttribute.isDefault())
            {
                const_cast < ViewContactOfE3dScene* >(this)->createSdrSceneAttribute();
            }

            return maSdrSceneAttribute;
        }

        const drawinglayer::attribute::SdrLightingAttribute& ViewContactOfE3dScene::getSdrLightingAttribute() const
        {
            if(maSdrLightingAttribute.isDefault())
            {
                const_cast < ViewContactOfE3dScene* >(this)->createSdrLightingAttribute();
            }

            return maSdrLightingAttribute;
        }

        drawinglayer::primitive3d::Primitive3DSequence ViewContactOfE3dScene::getAllPrimitive3DSequence() const
        {
            drawinglayer::primitive3d::Primitive3DSequence aAllPrimitive3DSequence;
            const sal_uInt32 nChildrenCount(GetObjectCount());

            // add children recursively. Do NOT start with (*this), this would create
            // a 3D transformPrimitive for the start scene. While this is theoretically not
            // a bad thing, for historical reasons the transformation of the outmost scene
            // is seen as part of the ViewTransformation (see text in createViewInformation3D)
            for(sal_uInt32 a(0L); a < nChildrenCount; a++)
            {
                createSubPrimitive3DVector(GetViewContact(a), aAllPrimitive3DSequence, 0, 0, false);
            }

            return aAllPrimitive3DSequence;
        }

        basegfx::B3DRange ViewContactOfE3dScene::getAllContentRange3D() const
        {
            const drawinglayer::primitive3d::Primitive3DSequence xAllSequence(getAllPrimitive3DSequence());
            basegfx::B3DRange aAllContentRange3D;

            if(xAllSequence.hasElements())
            {
                // for getting the 3D range using getB3DRangeFromPrimitive3DSequence a ViewInformation3D
                // needs to be given for evtl. decompositions. Use a neutral ViewInformation3D here. This
                // leaves all matrices on identity and the time on 0.0.
                const uno::Sequence< beans::PropertyValue > aEmptyProperties;
                const drawinglayer::geometry::ViewInformation3D aNeutralViewInformation3D(aEmptyProperties);

                aAllContentRange3D = drawinglayer::primitive3d::getB3DRangeFromPrimitive3DSequence(xAllSequence, aNeutralViewInformation3D);
            }

            return aAllContentRange3D;
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
