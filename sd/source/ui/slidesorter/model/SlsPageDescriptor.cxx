/*************************************************************************
 *
 *  $RCSfile: SlsPageDescriptor.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:24:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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




view::PageObjectViewObjectContact* PageDescriptor::GetViewObjectContact (void)
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
