/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

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
                const basegfx::B3DHomMatrix& rSceneParentTransform = pSceneParent->GetE3dScene().GetB3DTransform();
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
            if(!mxViewIndependentPrimitive3DSequence.hasElements())
            {
                // create primitive list on demand
                const_cast< ViewContactOfE3d* >(this)->mxViewIndependentPrimitive3DSequence = createViewIndependentPrimitive3DSequence();
            }

            // return current Primitive2DSequence
            return mxViewIndependentPrimitive3DSequence;
        }

        // React on changes of the object of this ViewContact
        void ViewContactOfE3d::ActionChanged()
        {
            // call parent
            ViewContactOfSdrObj::ActionChanged();

            // reset local buffered primitive sequence (always)
            if(mxViewIndependentPrimitive3DSequence.hasElements())
            {
                mxViewIndependentPrimitive3DSequence.realloc(0);
            }
        }

        drawinglayer::primitive3d::Primitive3DSequence ViewContactOfE3d::getViewIndependentPrimitive3DSequence() const
        {
            // get sequence without object transform
            drawinglayer::primitive3d::Primitive3DSequence xRetval(getVIP3DSWithoutObjectTransform());

            if(xRetval.hasElements())
            {
                // add object transform if it's used
                const basegfx::B3DHomMatrix& rObjectTransform(GetE3dObject().GetB3DTransform());

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

        drawinglayer::attribute::SdrLineAttribute* ViewContactOfE3d::impCreateFallbackLineAttribute(const basegfx::BColor& rBColor) const
        {
            static bool bFallbackToCreateAsLineForTest(false);

            if(bFallbackToCreateAsLineForTest)
            {
                return new drawinglayer::attribute::SdrLineAttribute(rBColor);
            }
            else
            {
                return 0;
            }
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
