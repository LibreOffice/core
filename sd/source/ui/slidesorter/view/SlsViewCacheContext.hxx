/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsViewCacheContext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:45:13 $
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
