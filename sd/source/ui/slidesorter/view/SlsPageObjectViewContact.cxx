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

#include "precompiled_sd.hxx"

#include "view/SlsPageObjectViewContact.hxx"

#include "model/SlsPageDescriptor.hxx"
#include "controller/SlsPageObjectFactory.hxx"

#include <svx/svdopage.hxx>
#include <tools/debug.hxx>

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>

using namespace ::sdr::contact;

namespace sd { namespace slidesorter { namespace view {


PageObjectViewContact::PageObjectViewContact (
    SdrPageObj& rPageObj,
    const model::SharedPageDescriptor& rpDescriptor)
    : ViewContactOfPageObj (rPageObj),
      mbInDestructor(false),
      mpDescriptor(rpDescriptor)
{
}

PageObjectViewContact::~PageObjectViewContact (void)
{
    // remember that this instance is in destruction
    mbInDestructor = true;
}



ViewObjectContact&
    PageObjectViewContact::CreateObjectSpecificViewObjectContact(
        ObjectContact& rObjectContact)
{
    OSL_ASSERT(mpDescriptor.get()!=NULL);

    ViewObjectContact* pResult
        = mpDescriptor->GetPageObjectFactory().CreateViewObjectContact (
            rObjectContact,
            *this);
    DBG_ASSERT (pResult!=NULL,
        "PageObjectViewContact::CreateObjectSpecificViewObjectContact() was not able to create object.");
    return *pResult;
}

const SdrPage* PageObjectViewContact::GetPage (void) const
{
    // when this instance itself is in destruction, do no longer
    // provide the referenced page to VOC childs of this OC. This
    // happens e.g. in destructor which destroys all child-VOCs which
    // may in their implementation still reference their VC from
    // their own destructor
    if (!mbInDestructor)
        return GetReferencedPage();
    else
        return NULL;
}

void PageObjectViewContact::ActionChanged (void)
{
    ViewContactOfPageObj::ActionChanged();
}

SdrPageObj& PageObjectViewContact::GetPageObject (void) const
{
    return ViewContactOfPageObj::GetPageObj();
}

drawinglayer::primitive2d::Primitive2DSequence PageObjectViewContact::createViewIndependentPrimitive2DSequence() const
{
    // ceate graphical visualisation data. Since this is the view-independent version which should not be used,
    // create a replacement graphic visualisation here. Use GetLastBoundRect to access the model data directly
    // which is aOutRect for SdrPageObj.
    OSL_ASSERT(mpDescriptor.get()!=NULL);
    Rectangle aModelRectangle(GetPageObj().GetLastBoundRect());
    const SvBorder aBorder(mpDescriptor->GetModelBorder());

    aModelRectangle.Left() -= aBorder.Left();
    aModelRectangle.Right() += aBorder.Right();
    aModelRectangle.Top() -= aBorder.Top();
    aModelRectangle.Bottom() += aBorder.Bottom();

    const basegfx::B2DRange aModelRange(aModelRectangle.Left(), aModelRectangle.Top(), aModelRectangle.Right(), aModelRectangle.Bottom());
    const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(aModelRange));
    const basegfx::BColor aYellow(1.0, 1.0, 0.0);
    const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aOutline, aYellow));

    return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
}

} } } // end of namespace ::sd::slidesorter::view

// eof
