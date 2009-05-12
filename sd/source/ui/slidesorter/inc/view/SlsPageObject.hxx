/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsPageObject.hxx,v $
 * $Revision: 1.5 $
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

#ifndef SD_SLIDESORTER_PAGE_OBJECT_HXX
#define SD_SLIDESORTER_PAGE_OBJECT_HXX

#include "model/SlsSharedPageDescriptor.hxx"

#include <svx/svdopage.hxx>

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
        const model::SharedPageDescriptor& rpDescriptor);

    virtual ~PageObject (void);

    model::SharedPageDescriptor GetDescriptor (void) const;

private:
    model::SharedPageDescriptor mpDescriptor;

    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact (void);
};



} } } // end of namespace ::sd::slidesorter::view

#endif
