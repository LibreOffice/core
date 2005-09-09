/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageObject.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:22:33 $
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

#ifndef SD_SLIDESORTER_PAGE_OBJECT_HXX
#define SD_SLIDESORTER_PAGE_OBJECT_HXX

#include <svx/svdopage.hxx>

namespace sd { namespace slidesorter { namespace model {
class PageDescriptor;
} } }

namespace sd { namespace slidesorter { namespace view {


/** This sub class of the SdrPageObject exists to create the view contact
    and indirectly to create view-object-contact objects.
*/
class PageObject
    : public SdrPageObj
{
public:
    PageObject (
        const Rectangle& rRect,
        SdrPage* pPage,
        model::PageDescriptor& rDescriptor);

    virtual ~PageObject (void);

    model::PageDescriptor& GetDescriptor (void) const;

private:
    model::PageDescriptor& mrDescriptor;

    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact (void);
};



} } } // end of namespace ::sd::slidesorter::view

#endif
