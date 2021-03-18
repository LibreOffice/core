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


#include <sdr/contact/viewobjectcontactofmasterpagedescriptor.hxx>
#include <sdr/contact/viewcontactofmasterpagedescriptor.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/svdpage.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>


namespace sdr::contact
{
        ViewObjectContactOfMasterPageDescriptor::ViewObjectContactOfMasterPageDescriptor(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   ViewObjectContact(rObjectContact, rViewContact)
        {
        }

        ViewObjectContactOfMasterPageDescriptor::~ViewObjectContactOfMasterPageDescriptor()
        {
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

        drawinglayer::primitive2d::Primitive2DContainer ViewObjectContactOfMasterPageDescriptor::getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const
        {
            drawinglayer::primitive2d::Primitive2DContainer xRetval;
            drawinglayer::primitive2d::Primitive2DContainer xMasterPageSequence;
            const sdr::MasterPageDescriptor& rDescriptor = static_cast< ViewContactOfMasterPageDescriptor& >(GetViewContact()).GetMasterPageDescriptor();

            // used range (retval) is fixed here, it's the MasterPage fill range
            const SdrPage& rOwnerPage = rDescriptor.GetOwnerPage();
            const basegfx::B2DRange aInnerRange(
                rOwnerPage.GetLeftBorder(), rOwnerPage.GetUpperBorder(),
                rOwnerPage.GetWidth() - rOwnerPage.GetRightBorder(), rOwnerPage.GetHeight() - rOwnerPage.GetLowerBorder());
            const basegfx::B2DRange aOuterRange(
                0, 0, rOwnerPage.GetWidth(), rOwnerPage.GetHeight());
            // ??? somehow only the master page's bit is used
            bool const isFullSize(rDescriptor.GetUsedPage().IsBackgroundFullSize());
            basegfx::B2DRange const& rPageFillRange(isFullSize ? aOuterRange : aInnerRange);

            // Modify DisplayInfo for MasterPageContent collection; remember original layers and
            // set combined SdrLayerIDSet; set MasterPagePaint flag
            const SdrLayerIDSet aRememberedLayers(rDisplayInfo.GetProcessLayers());
            SdrLayerIDSet aPreprocessedLayers(aRememberedLayers);
            aPreprocessedLayers &= rDescriptor.GetVisibleLayers();
            rDisplayInfo.SetProcessLayers(aPreprocessedLayers);
            rDisplayInfo.SetSubContentActive(true);

            // check layer visibility (traditionally was member of layer 1)
            if(aPreprocessedLayers.IsSet(SdrLayerID(1)))
            {
                // hide PageBackground for special DrawModes; historical reasons
                if(!GetObjectContact().isDrawModeGray() && !GetObjectContact().isDrawModeHighContrast())
                {
                    // if visible, create the default background primitive sequence
                    xRetval = static_cast< ViewContactOfMasterPageDescriptor& >(GetViewContact()).getViewIndependentPrimitive2DContainer();
                }
            }

            // hide MasterPage content? Test self here for hierarchy
            if(isPrimitiveVisible(rDisplayInfo))
            {
                // get the VOC of the Master-SdrPage and get its object hierarchy
                ViewContact& rViewContactOfMasterPage(rDescriptor.GetUsedPage().GetViewContact());
                ViewObjectContact& rVOCOfMasterPage(rViewContactOfMasterPage.GetViewObjectContact(GetObjectContact()));

                xMasterPageSequence = rVOCOfMasterPage.getPrimitive2DSequenceHierarchy(rDisplayInfo);
            }

            // reset DisplayInfo changes for MasterPage paint
            rDisplayInfo.SetProcessLayers(aRememberedLayers);
            rDisplayInfo.SetSubContentActive(false);

            if(!xMasterPageSequence.empty())
            {
                // get range of MasterPage sub hierarchy
                const drawinglayer::geometry::ViewInformation2D& rViewInformation2D(GetObjectContact().getViewInformation2D());
                const basegfx::B2DRange aSubHierarchyRange(xMasterPageSequence.getB2DRange(rViewInformation2D));

                if (rPageFillRange.isInside(aSubHierarchyRange))
                {
                    // completely inside, just render MasterPage content. Add to target
                    xRetval.append(xMasterPageSequence);
                }
                else if (rPageFillRange.overlaps(aSubHierarchyRange))
                {
                    // overlapping, compute common area
                    basegfx::B2DRange aCommonArea(rPageFillRange);
                    aCommonArea.intersect(aSubHierarchyRange);

                    // need to create a clip primitive, add clipped list to target
                    const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::MaskPrimitive2D(
                        basegfx::B2DPolyPolygon(basegfx::utils::createPolygonFromRect(aCommonArea)), xMasterPageSequence));
                    xRetval.push_back(xReference);
                }
            }

            // return grouped primitive
            return xRetval;
        }
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
