/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageObjectViewContact.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:44:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#include "view/SlsPageObjectViewContact.hxx"

#include "model/SlsPageDescriptor.hxx"
#include "controller/SlsPageObjectFactory.hxx"

#include <svx/svdopage.hxx>

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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
