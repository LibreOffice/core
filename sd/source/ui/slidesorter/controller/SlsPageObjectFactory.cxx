/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageObjectFactory.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-24 07:43:03 $
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

#include "controller/SlsPageObjectFactory.hxx"

#include "view/SlsPageObject.hxx"
#include "view/SlsPageObjectViewContact.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"

#include "sdpage.hxx"


namespace sd { namespace slidesorter { namespace controller {


PageObjectFactory::PageObjectFactory (const ::boost::shared_ptr<cache::PageCache>& rpCache)
    : mpPageCache(rpCache)
{
}




PageObjectFactory::~PageObjectFactory (void)
{
}




view::PageObject* PageObjectFactory::CreatePageObject (
    SdPage* pPage,
    model::PageDescriptor& rDescriptor) const
{
    return new view::PageObject(
        Rectangle (Point(0,0), pPage->GetSize()),
        pPage,
        rDescriptor);
}




::sdr::contact::ViewContact*
    PageObjectFactory::CreateViewContact (
        view::PageObject* pPageObject,
        model::PageDescriptor& rDescriptor) const
{
    return new view::PageObjectViewContact (
        *pPageObject,
        rDescriptor);
}




::sdr::contact::ViewObjectContact*
    PageObjectFactory::CreateViewObjectContact (
        ::sdr::contact::ObjectContact& rObjectContact,
        ::sdr::contact::ViewContact& rViewContact) const
{
    return new view::PageObjectViewObjectContact (
        rObjectContact,
        rViewContact,
        mpPageCache);
}


} } } // end of namespace ::sd::slidesorter::controller
