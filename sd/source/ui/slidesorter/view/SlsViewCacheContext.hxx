/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsViewCacheContext.hxx,v $
 *
 * $Revision: 1.3 $
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

#ifndef SD_SLIDESORTER_VIEW_VIEW_CACHE_CONTEXT_HXX
#define SD_SLIDESORTER_VIEW_VIEW_CACHE_CONTEXT_HXX

#include "cache/SlsCacheContext.hxx"
#include "model/SlsSharedPageDescriptor.hxx"

namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
} } }

namespace sd { namespace slidesorter { namespace view {

class SlideSorterView;

/** The cache context for the SlideSorter as used by Draw and Impress.  See
    the base class for documentation of the individual methods.
*/
class ViewCacheContext : public cache::CacheContext
{
public:
    ViewCacheContext (
        model::SlideSorterModel& rModel,
        SlideSorterView& rView);
    virtual ~ViewCacheContext (void);
    virtual void NotifyPreviewCreation (cache::CacheKey aKey, const ::boost::shared_ptr<BitmapEx>& rPreview);
    virtual bool IsIdle (void);
    virtual bool IsVisible (cache::CacheKey aKey);
    virtual const SdrPage* GetPage (cache::CacheKey aKey);
    virtual ::boost::shared_ptr<std::vector<cache::CacheKey> > GetEntryList (bool bVisible);
    virtual sal_Int32 GetPriority (cache::CacheKey aKey);
    virtual ::com::sun::star::uno::Reference<com::sun::star::uno::XInterface> GetModel (void);

private:
    model::SlideSorterModel& mrModel;
    SlideSorterView& mrView;

    model::SharedPageDescriptor GetDescriptor (cache::CacheKey aKey);
};


} } } // end of namespace ::sd::slidesorter::view

#endif
