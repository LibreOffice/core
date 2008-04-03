/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageDescriptor.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:41:06 $
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




sal_Int32 PageDescriptor::GetPageIndex (void) const
{
    return mnIndex;
}




view::PageObject* PageDescriptor::GetPageObject (void)
{
    if (mpPageObject==NULL && mpPageObjectFactory!=NULL)
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




bool PageDescriptor::IsCurrentPage (void) const
{
    return mbIsCurrent;
}




void PageDescriptor::SetIsCurrentPage (const bool bIsCurrent)
{
    mbIsCurrent = bIsCurrent;
}



} } } // end of namespace ::sd::slidesorter::model
