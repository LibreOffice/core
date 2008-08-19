/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofe3d.cxx,v $
 *
 * $Revision: 1.2 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/sdr/contact/viewcontactofe3d.hxx>
#include <svx/sdr/contact/viewobjectcontactofe3d.hxx>
#include <svx/obj3d.hxx>
#include <drawinglayer/primitive2d/embedded3dprimitive2d.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <drawinglayer/attribute/sdrattribute.hxx>
#include <drawinglayer/attribute/sdrattribute3d.hxx>

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
                // try to get the ViewObjectContactOfE3dScene for this single 3d object
                const ViewContactOfE3dScene* pVCOfE3DScene = tryToFindVCOfE3DScene();

                if(pVCOfE3DScene)
                {
                    basegfx::B3DVector aLightNormal;
                    const double fShadowSlant(pVCOfE3DScene->getSdrSceneAttribute().getShadowSlant());
                    const basegfx::B3DRange& rAllContentRange = pVCOfE3DScene->getAllContentRange3D();

                    if(pVCOfE3DScene->getSdrLightingAttribute().getLightVector().size())
                    {
                        // get light normal from first light and normalize
                        aLightNormal = pVCOfE3DScene->getSdrLightingAttribute().getLightVector()[0].getDirection();
                        aLightNormal.normalize();
                    }

                    // create embedded 2d primitive and add. LightNormal and ShadowSlant are needed for evtl.
                    // 3D shadow extraction for correct B2DRange calculation (shadow is part of the object)
                    const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::Embedded3DPrimitive2D(
                        rxContent3D,
                        pVCOfE3DScene->getObjectTransformation(),
                        pVCOfE3DScene->getViewInformation3D(),
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

        drawinglayer::primitive3d::Primitive3DSequence ViewContactOfE3d::getViewIndependentPrimitive3DSequence() const
        {
            // local up-to-date checks. Create new list and compare.
            const drawinglayer::primitive3d::Primitive3DSequence xNew(createViewIndependentPrimitive3DSequence());

            if(!drawinglayer::primitive3d::arePrimitive3DSequencesEqual(mxViewIndependentPrimitive3DSequence, xNew))
            {
                // has changed, copy content
                const_cast< ViewContactOfE3d* >(this)->mxViewIndependentPrimitive3DSequence = xNew;
            }

            // return current Primitive2DSequence
            return mxViewIndependentPrimitive3DSequence;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfE3d::createViewIndependentPrimitive2DSequence() const
        {
            // also need to create a 2D embedding when the view-independent part is requested,
            // see view-dependent part in ViewObjectContactOfE3d::createPrimitive2DSequence
            // get 3d primitive vector, isPrimitiveVisible() is done in 3d creator
            return impCreateWithGivenPrimitive3DSequence(getViewIndependentPrimitive3DSequence());
        }

        const ViewContactOfE3dScene* ViewContactOfE3d::tryToFindVCOfE3DScene() const
        {
            const ViewContact* pParent = GetParentContact();

            if(pParent)
            {
                const ViewContactOfE3dScene* pSceneParent = dynamic_cast< const ViewContactOfE3dScene* >(pParent);

                if(pSceneParent)
                {
                    return pSceneParent;
                }
                else
                {
                    const ViewContactOfE3d* p3dParent = dynamic_cast< const ViewContactOfE3d* >(pParent);

                    if(p3dParent)
                    {
                        return p3dParent->tryToFindVCOfE3DScene();
                    }
                }
            }

            return 0L;
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
                ::std::vector< double > aEmptyVector;
                return new drawinglayer::attribute::SdrLineAttribute(
                    basegfx::B2DLINEJOIN_NONE,
                    0.0,
                    0.0,
                    rBColor,
                    aEmptyVector,
                    0.0);
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
