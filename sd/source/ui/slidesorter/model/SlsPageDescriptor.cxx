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
#include "precompiled_sd.hxx"

#include "model/SlsPageDescriptor.hxx"
#include "view/SlsPageObject.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"
#include "controller/SlsPageObjectFactory.hxx"

#include "sdpage.hxx"
#include "drawdoc.hxx"

#include <svx/svdopage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdr/contact/viewcontact.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

namespace sd {  namespace slidesorter { namespace model {

PageDescriptor::PageDescriptor (
    const Reference<drawing::XDrawPage>& rxPage,
    SdPage* pPage,
    const sal_Int32 nIndex,
    const controller::PageObjectFactory& rPageObjectFactory)
    : mpPage(pPage),
      mxPage(rxPage),
      mnIndex(nIndex),
      mpPageObjectFactory(&rPageObjectFactory),
      mpPageObject(NULL),
      mbIsSelected(false),
      mbIsVisible(false),
      mbIsFocused(false),
      mbIsCurrent(false),
      mpViewObjectContact(NULL),
      maModelBorder(0,0,0,0),
      maPageNumberAreaModelSize(0,0)
{
    OSL_ASSERT(mpPage == SdPage::getImplementation(rxPage));
}




PageDescriptor::~PageDescriptor (void)
{
}




SdPage* PageDescriptor::GetPage (void) const
{
    return mpPage;
}




Reference<drawing::XDrawPage> PageDescriptor::GetXDrawPage (void) const
{
    return mxPage;
}




view::PageObject* PageDescriptor::GetPageObject (void)
{
    if (mpPageObject==NULL && mpPageObjectFactory!=NULL && mpPage != NULL)
    {
        mpPageObject = mpPageObjectFactory->CreatePageObject(mpPage, shared_from_this());
    }

    return mpPageObject;
}




void PageDescriptor::ReleasePageObject (void)
{
    mpPageObject = NULL;
}




bool PageDescriptor::IsVisible (void) const
{
    return mbIsVisible;
}




void PageDescriptor::SetVisible (bool bIsVisible)
{
    mbIsVisible = bIsVisible;
}




bool PageDescriptor::Select (void)
{
    if ( ! IsSelected())
    {
        mbIsSelected = true;
        return true;
    }
    else
        return false;
}




bool PageDescriptor::Deselect (void)
{
    if (IsSelected())
    {
        mbIsSelected = false;
        return true;
    }
    else
        return false;
}




bool PageDescriptor::IsSelected (void) const
{
    return mbIsSelected;
}




bool PageDescriptor::UpdateSelection (void)
{
    if (mpPage!=NULL && (mpPage->IsSelected()==TRUE) != mbIsSelected)
    {
        mbIsSelected = ! mbIsSelected;
        return true;
    }
    else
        return false;
}




bool PageDescriptor::IsFocused (void) const
{
    return mbIsFocused;
}




void PageDescriptor::SetFocus (void)
{
    mbIsFocused = true;
}




void PageDescriptor::RemoveFocus (void)
{
    mbIsFocused = false;
}




view::PageObjectViewObjectContact*
    PageDescriptor::GetViewObjectContact (void) const
{
    return mpViewObjectContact;
}




void PageDescriptor::SetViewObjectContact (
    view::PageObjectViewObjectContact* pViewObjectContact)
{
    mpViewObjectContact = pViewObjectContact;
}




const controller::PageObjectFactory&
    PageDescriptor::GetPageObjectFactory (void) const
{
    return *mpPageObjectFactory;
}




void PageDescriptor::SetPageObjectFactory (
    const controller::PageObjectFactory& rFactory)
{
    mpPageObjectFactory = &rFactory;
}




void PageDescriptor::SetModelBorder (const SvBorder& rBorder)
{
    maModelBorder = rBorder;
}




SvBorder PageDescriptor::GetModelBorder (void) const
{
    return maModelBorder;
}




void PageDescriptor::SetPageNumberAreaModelSize (const Size& rSize)
{
    maPageNumberAreaModelSize = rSize;
}




Size PageDescriptor::GetPageNumberAreaModelSize (void) const
{
    return maPageNumberAreaModelSize;
}




void PageDescriptor::SetIsCurrentPage (const bool bIsCurrent)
{
    mbIsCurrent = bIsCurrent;
}



} } } // end of namespace ::sd::slidesorter::model
