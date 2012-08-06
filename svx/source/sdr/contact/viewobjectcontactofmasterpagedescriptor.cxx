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


#include <svx/sdr/contact/viewobjectcontactofmasterpagedescriptor.hxx>
#include <svx/sdr/contact/viewcontactofmasterpagedescriptor.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpage.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContactOfMasterPageDescriptor::ViewObjectContactOfMasterPageDescriptor(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   ViewObjectContact(rObjectContact, rViewContact)
        {
        }

        ViewObjectContactOfMasterPageDescriptor::~ViewObjectContactOfMasterPageDescriptor()
        {
        }

        sdr::MasterPageDescriptor& ViewObjectContactOfMasterPageDescriptor::GetMasterPageDescriptor() const
        {
            return static_cast< ViewContactOfMasterPageDescriptor& >(GetViewContact()).GetMasterPageDescriptor();
        }

        bool ViewObjectContactOfMasterPageDescriptor::isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const
        {
            if(rDisplayInfo.GetControlLayerProcessingActive())
            {
                return false;
            }

            if(!rDisplayInfo.GetPageProcessingActive())
            {
                return false;
            }

            return true;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfMasterPageDescriptor::getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            drawinglayer::primitive2d::Primitive2DSequence xMasterPageSequence;
            const sdr::MasterPageDescriptor& rDescriptor = GetMasterPageDescriptor();

            // used range (retval) is fixed here, it's the MasterPage fill range
            const SdrPage& rOwnerPage = rDescriptor.GetOwnerPage();
            const basegfx::B2DRange aPageFillRange(
                rOwnerPage.GetLftBorder(), rOwnerPage.GetUppBorder(),
                rOwnerPage.GetWdt() - rOwnerPage.GetRgtBorder(), rOwnerPage.GetHgt() - rOwnerPage.GetLwrBorder());

            // Modify DisplayInfo for MasterPageContent collection; remember original layers and
            // set combined LayerSet; set MasterPagePaint flag
            const SetOfByte aRememberedLayers(rDisplayInfo.GetProcessLayers());
            SetOfByte aPreprocessedLayers(aRememberedLayers);
            aPreprocessedLayers &= rDescriptor.GetVisibleLayers();
            rDisplayInfo.SetProcessLayers(aPreprocessedLayers);
            rDisplayInfo.SetSubContentActive(true);

            // check layer visibility (traditionally was member of layer 1)
            if(aPreprocessedLayers.IsSet(1))
            {
                // hide PageBackground for special DrawModes; historical reasons
                if(!GetObjectContact().isDrawModeGray() && !GetObjectContact().isDrawModeHighContrast())
                {
                    // if visible, create the default background primitive sequence
                    xRetval = static_cast< ViewContactOfMasterPageDescriptor& >(GetViewContact()).getViewIndependentPrimitive2DSequence();
                }
            }

            // hide MasterPage content? Test self here for hierarchy
            if(isPrimitiveVisible(rDisplayInfo))
            {
                // get the VOC of the Master-SdrPage and get it's object hierarchy
                ViewContact& rViewContactOfMasterPage(rDescriptor.GetUsedPage().GetViewContact());
                ViewObjectContact& rVOCOfMasterPage(rViewContactOfMasterPage.GetViewObjectContact(GetObjectContact()));

                xMasterPageSequence = rVOCOfMasterPage.getPrimitive2DSequenceHierarchy(rDisplayInfo);
            }

            // reset DisplayInfo changes for MasterPage paint
            rDisplayInfo.SetProcessLayers(aRememberedLayers);
            rDisplayInfo.SetSubContentActive(false);

            if(xMasterPageSequence.hasElements())
            {
                // get range of MasterPage sub hierarchy
                const drawinglayer::geometry::ViewInformation2D& rViewInformation2D(GetObjectContact().getViewInformation2D());
                const basegfx::B2DRange aSubHierarchyRange(drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(xMasterPageSequence, rViewInformation2D));

                if(aPageFillRange.isInside(aSubHierarchyRange))
                {
                    // completely inside, just render MasterPage content. Add to target
                    drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval, xMasterPageSequence);
                }
                else if(aPageFillRange.overlaps(aSubHierarchyRange))
                {
                    // overlapping, compute common area
                    basegfx::B2DRange aCommonArea(aPageFillRange);
                    aCommonArea.intersect(aSubHierarchyRange);

                    // need to create a clip primitive, add clipped list to target
                    const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::MaskPrimitive2D(
                        basegfx::B2DPolyPolygon(basegfx::tools::createPolygonFromRect(aCommonArea)), xMasterPageSequence));
                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(xRetval, xReference);
                }
            }

            // return grouped primitive
            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
