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
#include <svx/svdpage.hxx>
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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
