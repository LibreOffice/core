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


#include <svx/sdr/contact/viewcontactofe3d.hxx>
#include <svx/sdr/contact/viewobjectcontactofe3d.hxx>
#include <svx/obj3d.hxx>
#include <drawinglayer/primitive2d/embedded3dprimitive2d.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <svx/scene3d.hxx>
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <drawinglayer/attribute/sdrsceneattribute3d.hxx>
#include <drawinglayer/attribute/sdrlightingattribute3d.hxx>
#include <drawinglayer/attribute/sdrlightattribute3d.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace
{
    const sdr::contact::ViewContactOfE3dScene* tryToFindVCOfE3DScene(
        const sdr::contact::ViewContact& rCandidate,
        basegfx::B3DHomMatrix& o_rInBetweenObjectTransform)
    {
        const sdr::contact::ViewContactOfE3dScene* pSceneParent =
            dynamic_cast< const sdr::contact::ViewContactOfE3dScene* >(rCandidate.GetParentContact());

        if(pSceneParent)
        {
            // each 3d object (including in-between scenes) should have a scene as parent
            const sdr::contact::ViewContactOfE3dScene* pSceneParentParent =
                dynamic_cast< const sdr::contact::ViewContactOfE3dScene* >(pSceneParent->GetParentContact());

            if(pSceneParentParent)
            {
                // the parent scene of rCandidate is a in-between scene, call recursively and collect
                // the in-between scene's object transformation part in o_rInBetweenObjectTransform
                const basegfx::B3DHomMatrix& rSceneParentTransform = pSceneParent->GetE3dScene().GetTransform();
                o_rInBetweenObjectTransform = rSceneParentTransform * o_rInBetweenObjectTransform;
                return tryToFindVCOfE3DScene(*pSceneParent, o_rInBetweenObjectTransform);
            }
            else
            {
                // the parent scene is the outmost scene
                return pSceneParent;
            }
        }

        // object hierarchy structure is incorrect; no result
        return 0;
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfE3d::impCreateWithGivenPrimitive3DSequence(
            const drawinglayer::primitive3d::Primitive3DSequence& rxContent3D) const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            if(rxContent3D.hasElements())
            {
                // try to get the outmost ViewObjectContactOfE3dScene for this single 3d object,
                // the ones on the way there are grouping scenes. Collect the in-between scene's
                // transformations to build a correct object transformation for the embedded
                // object
                basegfx::B3DHomMatrix aInBetweenObjectTransform;
                const ViewContactOfE3dScene* pVCOfE3DScene = tryToFindVCOfE3DScene(*this, aInBetweenObjectTransform);

                if(pVCOfE3DScene)
                {
                    basegfx::B3DVector aLightNormal;
                    const double fShadowSlant(pVCOfE3DScene->getSdrSceneAttribute().getShadowSlant());
                    const basegfx::B3DRange& rAllContentRange = pVCOfE3DScene->getAllContentRange3D();
                    drawinglayer::geometry::ViewInformation3D aViewInformation3D(pVCOfE3DScene->getViewInformation3D());

                    if(pVCOfE3DScene->getSdrLightingAttribute().getLightVector().size())
                    {
                        // get light normal from first light and normalize
                        aLightNormal = pVCOfE3DScene->getSdrLightingAttribute().getLightVector()[0].getDirection();
                        aLightNormal.normalize();
                    }

                    if(!aInBetweenObjectTransform.isIdentity())
                    {
                        // if aInBetweenObjectTransform is used, create combined ViewInformation3D which
                        // contains the correct object transformation for the embedded 3d object
                        aViewInformation3D = drawinglayer::geometry::ViewInformation3D(
                            aViewInformation3D.getObjectTransformation() * aInBetweenObjectTransform,
                            aViewInformation3D.getOrientation(),
                            aViewInformation3D.getProjection(),
                            aViewInformation3D.getDeviceToView(),
                            aViewInformation3D.getViewTime(),
                            aViewInformation3D.getExtendedInformationSequence());
                    }

                    // create embedded 2d primitive and add. LightNormal and ShadowSlant are needed for evtl.
                    // 3D shadow extraction for correct B2DRange calculation (shadow is part of the object)
                    const drawinglayer::primitive2d::Primitive2DReference xReference(
                        new drawinglayer::primitive2d::Embedded3DPrimitive2D(
                            rxContent3D,
                            pVCOfE3DScene->getObjectTransformation(),
                            aViewInformation3D,
                            aLightNormal,
                            fShadowSlant,
                            rAllContentRange));

                    xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                }
            }

            return xRetval;
        }

        ViewContactOfE3d::ViewContactOfE3d(E3dObject& rSdrObject)
        :   ViewContactOfSdrObj(rSdrObject)
        {
        }

        ViewContactOfE3d::~ViewContactOfE3d()
        {
        }

        drawinglayer::primitive3d::Primitive3DSequence ViewContactOfE3d::getVIP3DSWithoutObjectTransform() const
        {
            // local up-to-date checks. Create new list and compare.
            drawinglayer::primitive3d::Primitive3DSequence xNew(createViewIndependentPrimitive3DSequence());

            if(!drawinglayer::primitive3d::arePrimitive3DSequencesEqual(mxViewIndependentPrimitive3DSequence, xNew))
            {
                // has changed, copy content
                const_cast< ViewContactOfE3d* >(this)->mxViewIndependentPrimitive3DSequence = xNew;
            }

            // return current Primitive2DSequence
            return mxViewIndependentPrimitive3DSequence;
        }

        drawinglayer::primitive3d::Primitive3DSequence ViewContactOfE3d::getViewIndependentPrimitive3DSequence() const
        {
            // get sequence without object transform
            drawinglayer::primitive3d::Primitive3DSequence xRetval(getVIP3DSWithoutObjectTransform());

            if(xRetval.hasElements())
            {
                // add object transform if it's used
                const basegfx::B3DHomMatrix& rObjectTransform(GetE3dObject().GetTransform());

                if(!rObjectTransform.isIdentity())
                {
                    const drawinglayer::primitive3d::Primitive3DReference xReference(
                        new drawinglayer::primitive3d::TransformPrimitive3D(
                            rObjectTransform,
                            xRetval));

                    xRetval = drawinglayer::primitive3d::Primitive3DSequence(&xReference, 1);
                }
            }

            // return current Primitive2DSequence
            return xRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfE3d::createViewIndependentPrimitive2DSequence() const
        {
            // also need to create a 2D embedding when the view-independent part is requested,
            // see view-dependent part in ViewObjectContactOfE3d::createPrimitive2DSequence
            // get 3d primitive vector, isPrimitiveVisible() is done in 3d creator
            return impCreateWithGivenPrimitive3DSequence(getViewIndependentPrimitive3DSequence());
        }

        ViewObjectContact& ViewContactOfE3d::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfE3d(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContactOfE3d::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
