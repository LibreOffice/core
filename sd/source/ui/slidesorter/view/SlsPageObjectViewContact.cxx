/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsPageObjectViewContact.cxx,v $
 * $Revision: 1.9 $
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

using namespace ::sdr::contact;

namespace sd { namespace slidesorter { namespace view {


PageObjectViewContact::PageObjectViewContact (
    SdrPageObj& rPageObj,
    const model::SharedPageDescriptor& rpDescriptor)
    : ViewContactOfPageObj (rPageObj),
      mbIsValid(true),
      mpDescriptor(rpDescriptor)
{
}




PageObjectViewContact::~PageObjectViewContact (void)
{
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
    if (mbIsValid)
        return GetReferencedPage();
    else
        return NULL;
}




void PageObjectViewContact::CalcPaintRectangle (void)
{
    ViewContactOfPageObj::CalcPaintRectangle();
    if (mbIsValid)
    {
        OSL_ASSERT(mpDescriptor.get()!=NULL);

        maPageObjectBoundingBox = maPaintRectangle;
        SvBorder aBorder (mpDescriptor->GetModelBorder());
        maPaintRectangle.Left() -= aBorder.Left();
        maPaintRectangle.Right() += aBorder.Right();
        maPaintRectangle.Top() -= aBorder.Top();
        maPaintRectangle.Bottom() += aBorder.Bottom();
    }
}




Rectangle PageObjectViewContact::GetPageRectangle (void)
{
    return GetPageObj().GetCurrentBoundRect();
}




void PageObjectViewContact::ActionChanged (void)
{
    ViewContactOfPageObj::ActionChanged();
}




Rectangle PageObjectViewContact::GetPageObjectBoundingBox (void) const
{
    return maPageObjectBoundingBox;
}




SdrPageObj& PageObjectViewContact::GetPageObject (void) const
{
    return ViewContactOfPageObj::GetPageObj();
}




void PageObjectViewContact::PrepareDelete (void)
{
    mbIsValid = false;
}



} } } // end of namespace ::sd::slidesorter::view
