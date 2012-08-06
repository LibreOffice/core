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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
