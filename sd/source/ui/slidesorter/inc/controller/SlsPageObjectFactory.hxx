/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SD_SLIDESORTER_PAGE_OBJECT_FACTORY_HEADER
#define SD_SLIDESORTER_PAGE_OBJECT_FACTORY_HEADER

#include <boost/shared_ptr.hpp>
#include "model/SlsSharedPageDescriptor.hxx"

class SdPage;

namespace sdr { namespace contact {
class ObjectContact;
class ViewContact;
class ViewObjectContact;
} }

namespace sd { namespace slidesorter { namespace view {
class PageObject;
} } }

namespace sd { namespace slidesorter { namespace cache {
class PageCache;
} } }


namespace sd { namespace slidesorter { namespace controller {

class Properties;

/** This class is a factory for the creation of objects that represent page
    objects (shapes).  This includes the contact objects of the drawing
    layer.

    <p>The factory methods are called by the model::PageDescriptor and the
    standard implementations of the contact objects.</p>

    <p>The factory forwars the preview cache and Properties object to page
    objects and contact objects.</p>
*/
class PageObjectFactory
{
public:
    /** Create a new PageObjectFactory object that has references to the
        given cache and properties.
    */
    PageObjectFactory (
        const ::boost::shared_ptr<cache::PageCache>& rpCache,
        const ::boost::shared_ptr<controller::Properties>& rpProperties);
    PageObjectFactory (const ::boost::shared_ptr<cache::PageCache>& rpCache);
    virtual ~PageObjectFactory (void);

    virtual view::PageObject* CreatePageObject (
        SdPage* pPage,
        const model::SharedPageDescriptor& rpDescriptor) const;

    virtual ::sdr::contact::ViewContact* CreateViewContact (
        view::PageObject* pPageObject,
        const model::SharedPageDescriptor& rpDescriptor) const;

    virtual ::sdr::contact::ViewObjectContact* CreateViewObjectContact (
        ::sdr::contact::ObjectContact& rObjectContact,
        ::sdr::contact::ViewContact& rViewContact) const;

private:
    ::boost::shared_ptr<cache::PageCache> mpPageCache;
    ::boost::shared_ptr<controller::Properties> mpProperties;
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
