/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageObjectFactory.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:26:26 $
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

#include "controller/SlsPageObjectFactory.hxx"

#include "view/SlsPageObject.hxx"
#include "view/SlsPageObjectViewContact.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"

#include "sdpage.hxx"


namespace sd { namespace slidesorter { namespace controller {


PageObjectFactory::PageObjectFactory (
    const ::boost::shared_ptr<cache::PageCache>& rpCache,
    const ::boost::shared_ptr<controller::Properties>& rpProperties)
    : mpPageCache(rpCache),
      mpProperties(rpProperties)
{
}




PageObjectFactory::~PageObjectFactory (void)
{
}




view::PageObject* PageObjectFactory::CreatePageObject (
    SdPage* pPage,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    return new view::PageObject(
        Rectangle (Point(0,0), pPage->GetSize()),
        pPage,
        rpDescriptor);
}




::sdr::contact::ViewContact*
    PageObjectFactory::CreateViewContact (
        view::PageObject* pPageObject,
        const model::SharedPageDescriptor& rpDescriptor) const
{
    return new view::PageObjectViewContact (
        *pPageObject,
        rpDescriptor);
}




::sdr::contact::ViewObjectContact*
    PageObjectFactory::CreateViewObjectContact (
        ::sdr::contact::ObjectContact& rObjectContact,
        ::sdr::contact::ViewContact& rViewContact) const
{
    return new view::PageObjectViewObjectContact (
        rObjectContact,
        rViewContact,
        mpPageCache,
        mpProperties);
}


} } } // end of namespace ::sd::slidesorter::controller
