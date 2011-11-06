/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SD_SLIDESORTER_VIEW_VIEW_CACHE_CONTEXT_HXX
#define SD_SLIDESORTER_VIEW_VIEW_CACHE_CONTEXT_HXX

#include "cache/SlsCacheContext.hxx"
#include "model/SlsSharedPageDescriptor.hxx"

namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
} } }

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace view {

/** The cache context for the SlideSorter as used by Draw and Impress.  See
    the base class for documentation of the individual methods.
*/
class ViewCacheContext : public cache::CacheContext
{
public:
    ViewCacheContext (SlideSorter& rSlideSorter);
    virtual ~ViewCacheContext (void);
    virtual void NotifyPreviewCreation (cache::CacheKey aKey, const Bitmap& rPreview);
    virtual bool IsIdle (void);
    virtual bool IsVisible (cache::CacheKey aKey);
    virtual const SdrPage* GetPage (cache::CacheKey aKey);
    virtual ::boost::shared_ptr<std::vector<cache::CacheKey> > GetEntryList (bool bVisible);
    virtual sal_Int32 GetPriority (cache::CacheKey aKey);
    virtual ::com::sun::star::uno::Reference<com::sun::star::uno::XInterface> GetModel (void);

private:
    model::SlideSorterModel& mrModel;
    SlideSorter& mrSlideSorter;

    model::SharedPageDescriptor GetDescriptor (cache::CacheKey aKey);
};


} } } // end of namespace ::sd::slidesorter::view

#endif
