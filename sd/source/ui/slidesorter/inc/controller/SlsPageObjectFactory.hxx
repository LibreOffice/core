/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageObjectFactory.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-24 07:43:36 $
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

#ifndef SD_SLIDESORTER_PAGE_OBJECT_FACTORY_HEADER
#define SD_SLIDESORTER_PAGE_OBJECT_FACTORY_HEADER

#include <boost/shared_ptr.hpp>

class SdPage;

namespace sdr { namespace contact {
class ObjectContact;
class ViewContact;
class ViewObjectContact;
} }

namespace sd { namespace slidesorter { namespace model {
class PageDescriptor;
} } }

namespace sd { namespace slidesorter { namespace view {
class PageObject;
} } }

namespace sd { namespace slidesorter { namespace cache {
class PageCache;
} } }


namespace sd { namespace slidesorter { namespace controller {

/** This class is a factory for the creation of objects that represent page
    objects (shapes).  This includes the contact objects of the drawing
    layer.

    The factory methods are called by the model::PageDescriptor and the
    standard implementations of the contact objects.
*/
class PageObjectFactory
{
public:
    PageObjectFactory (const ::boost::shared_ptr<cache::PageCache>& rpCache);
    virtual ~PageObjectFactory (void);

    virtual view::PageObject* CreatePageObject (
        SdPage* pPage,
        model::PageDescriptor& rDescriptor) const;

    virtual ::sdr::contact::ViewContact* CreateViewContact (
        view::PageObject* pPageObject,
        model::PageDescriptor& rDescriptor) const;

    virtual ::sdr::contact::ViewObjectContact* CreateViewObjectContact (
        ::sdr::contact::ObjectContact& rObjectContact,
        ::sdr::contact::ViewContact& rViewContact) const;

private:
    ::boost::shared_ptr<cache::PageCache> mpPageCache;
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

