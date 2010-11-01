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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/sdr/contact/viewobjectcontactofe3dscene.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <basegfx/color/bcolormodifier.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <svx/sdr/contact/viewobjectcontactofe3d.hxx>
#include <basegfx/tools/canvastools.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace
{
    // Helper method to recursively travel the DrawHierarchy for 3D objects contained in
    // the 2D Scene. This will chreate all VOCs for the currenbt OC which are needed
    // for ActionChanged() functionality
    void impInternalSubHierarchyTraveller(const sdr::contact::ViewObjectContact& rVOC)
    {
        const sdr::contact::ViewContact& rVC = rVOC.GetViewContact();
        const sal_uInt32 nSubHierarchyCount(rVC.GetObjectCount());

        for(sal_uInt32 a(0); a < nSubHierarchyCount; a++)
        {
            const sdr::contact::ViewObjectContact& rCandidate(rVC.GetViewContact(a).GetViewObjectContact(rVOC.GetObjectContact()));
            impInternalSubHierarchyTraveller(rCandidate);
        }
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContactOfE3dScene::ViewObjectContactOfE3dScene(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   ViewObjectContactOfSdrObj(rObjectContact, rViewContact)
        {
        }

        ViewObjectContactOfE3dScene::~ViewObjectContactOfE3dScene()
        {
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfE3dScene::createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
        {
            // handle ghosted, else the whole 3d group will be encapsulated to a ghosted primitive set (see below)
            const bool bHandleGhostedDisplay(GetObjectContact().DoVisualizeEnteredGroup() && !GetObjectContact().isOutputToPrinter() && rDisplayInfo.IsGhostedDrawModeActive());
            const bool bIsActiveVC(bHandleGhostedDisplay && GetObjectContact().getActiveViewContact() == &GetViewContact());

            if(bIsActiveVC)
            {
                // switch off ghosted, display contents normal
                const_cast< DisplayInfo& >(rDisplayInfo).ClearGhostedDrawMode();
            }

            // create 2d primitive with content, use layer visibility test
            // this uses no ghosted mode, so scenes in scenes and entering them will not
            // support ghosted for now. This is no problem currently but would need to be
            // added when sub-groups in 3d will be added one day.
            const ViewContactOfE3dScene& rViewContact = dynamic_cast< ViewContactOfE3dScene& >(GetViewContact());
            const SetOfByte& rVisibleLayers = rDisplayInfo.GetProcessLayers();
            drawinglayer::primitive2d::Primitive2DSequence xRetval(rViewContact.createScenePrimitive2DSequence(&rVisibleLayers));

            if(xRetval.hasElements())
            {
                // handle GluePoint
                if(!GetObjectContact().isOutputToPrinter() && GetObjectContact().AreGluePointsVisible())
                {
                    const drawinglayer::primitive2d::Primitive2DSequence xGlue(GetViewContact().createGluePointPrimitive2DSequence());

                    if(xGlue.hasElements())
                    {
                        drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval, xGlue);
                    }
                }

                // handle ghosted
                if(isPrimitiveGhosted(rDisplayInfo))
                {
                    const ::basegfx::BColor aRGBWhite(1.0, 1.0, 1.0);
                    const ::basegfx::BColorModifier aBColorModifier(aRGBWhite, 0.5, ::basegfx::BCOLORMODIFYMODE_INTERPOLATE);
                    const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::ModifiedColorPrimitive2D(xRetval, aBColorModifier));

                    xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                }
            }

            if(bIsActiveVC)
            {
                // set back, display ghosted again
                const_cast< DisplayInfo& >(rDisplayInfo).SetGhostedDrawMode();
            }

            return xRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfE3dScene::getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const
        {
            // To get the VOCs for the contained 3D objects created to get the correct
            // Draw hierarchy and ActionChanged() working properly, travel the DrawHierarchy
            // using a local tooling method
            impInternalSubHierarchyTraveller(*this);

            // call parent
            return ViewObjectContactOfSdrObj::getPrimitive2DSequenceHierarchy(rDisplayInfo);
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
