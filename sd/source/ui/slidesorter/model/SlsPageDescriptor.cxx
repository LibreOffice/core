/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageDescriptor.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 19:08:42 $
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

namespace sd {  namespace slidesorter { namespace model {

PageDescriptor::PageDescriptor (
    SdPage& rPage,
    const controller::PageObjectFactory& rPageObjectFactory)
    : mrPage (rPage),
      mpPageObjectFactory(&rPageObjectFactory),
      mpPageObject (NULL),
      mbIsSelected (false),
      mbVisible (false),
      mbFocused (false),
      mpViewObjectContact (NULL),
      maModelBorder (0,0,0,0),
      maPageNumberAreaModelSize(0,0)
{
}




PageDescriptor::~PageDescriptor (void)
{
}




SdPage* PageDescriptor::GetPage (void) const
{
    return &mrPage;
}




view::PageObject* PageDescriptor::GetPageObject (void)
{
    if (mpPageObject==NULL && mpPageObjectFactory!=NULL)
    {
        mpPageObject = mpPageObjectFactory->CreatePageObject(&mrPage, *this);
    }

    return mpPageObject;
}




void PageDescriptor::ReleasePageObject (void)
{
    mpPageObject = NULL;
}




bool PageDescriptor::IsVisible (void) const
{
    return mbVisible;
}




void PageDescriptor::SetVisible (bool bVisible)
{
    mbVisible = bVisible;
}




bool PageDescriptor::Select (void)
{
    if ( ! IsSelected())
    {
        mbIsSelected = true;
        //        mrPage.SetSelected (TRUE);
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
        //        mrPage.SetSelected (FALSE);
        return true;
    }
    else
        return false;
}




bool PageDescriptor::IsSelected (void) const
{
    return mbIsSelected;//mrPage.IsSelected();
}




bool PageDescriptor::UpdateSelection (void)
{
    if ((mrPage.IsSelected()==TRUE) != mbIsSelected)
    {
        mbIsSelected = ! mbIsSelected;
        return true;
    }
    else
        return false;
}




bool PageDescriptor::IsFocused (void) const
{
    return mbFocused;
}




void PageDescriptor::SetFocus (void)
{
    mbFocused = true;
}




void PageDescriptor::RemoveFocus (void)
{
    mbFocused = false;
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


} } } // end of namespace ::sd::slidesorter::model
