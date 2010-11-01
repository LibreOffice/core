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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
