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

#include <svx/sdr/contact/viewcontactofmasterpagedescriptor.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/svdobj.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <vcl/timer.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/sdr/contact/viewcontactofsdrpage.hxx>
#include <svx/sdr/contact/viewobjectcontactofmasterpagedescriptor.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContact& ViewContactOfMasterPageDescriptor::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            return *(new ViewObjectContactOfMasterPageDescriptor(rObjectContact, *this));
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfMasterPageDescriptor::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            drawinglayer::attribute::SdrFillAttribute aFill;
            const SdrPageProperties* pCorrectProperties = GetMasterPageDescriptor().getCorrectSdrPageProperties();

            if(pCorrectProperties)
            {
                // create page fill attributes when correct properties were identified
                aFill = drawinglayer::primitive2d::createNewSdrFillAttribute(pCorrectProperties->GetItemSet());
            }

            if(!aFill.isDefault())
            {
                // direct model data is the page size, get and use it
                const SdrPage& rOwnerPage = GetMasterPageDescriptor().GetOwnerPage();
                const basegfx::B2DRange aInnerRange(
                    rOwnerPage.GetLftBorder(), rOwnerPage.GetUppBorder(),
                    rOwnerPage.GetWdt() - rOwnerPage.GetRgtBorder(),
                    rOwnerPage.GetHgt() - rOwnerPage.GetLwrBorder());
                const basegfx::B2DPolygon aInnerPolgon(basegfx::tools::createPolygonFromRect(aInnerRange));
                const basegfx::B2DHomMatrix aEmptyTransform;
                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    drawinglayer::primitive2d::createPolyPolygonFillPrimitive(
                        basegfx::B2DPolyPolygon(aInnerPolgon),
                        aEmptyTransform,
                        aFill,
                        drawinglayer::attribute::FillGradientAttribute()));

                xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            }

            return xRetval;
        }

        // basic constructor
        ViewContactOfMasterPageDescriptor::ViewContactOfMasterPageDescriptor(sdr::MasterPageDescriptor& rDescriptor)
        :   ViewContact(),
            mrMasterPageDescriptor(rDescriptor)
        {
        }

        // The destructor.
        ViewContactOfMasterPageDescriptor::~ViewContactOfMasterPageDescriptor()
        {
        }

        sal_uInt32 ViewContactOfMasterPageDescriptor::GetObjectCount() const
        {
            return GetMasterPageDescriptor().GetUsedPage().GetObjCount();
        }

        ViewContact& ViewContactOfMasterPageDescriptor::GetViewContact(sal_uInt32 nIndex) const
        {
            return GetMasterPageDescriptor().GetUsedPage().GetObj(nIndex)->GetViewContact();
        }

        ViewContact* ViewContactOfMasterPageDescriptor::GetParentContact() const
        {
            return &(GetMasterPageDescriptor().GetOwnerPage().GetViewContact());
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
