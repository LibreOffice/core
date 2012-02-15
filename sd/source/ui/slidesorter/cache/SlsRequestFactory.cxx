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



#include "precompiled_sd.hxx"

#include "SlsRequestFactory.hxx"

#include "SlsGenericPageCache.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "view/SlideSorterView.hxx"
#include <svx/svdpagv.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdr/contact/viewcontact.hxx>


namespace sd { namespace slidesorter { namespace cache {

void RequestFactory::operator()(
    RequestQueue& rRequestQueue,
    const SharedCacheContext& rpCacheContext)
{
    ::boost::shared_ptr<std::vector<CacheKey> > aKeys;

    // Add the requests for the visible pages.
    aKeys = rpCacheContext->GetEntryList(true);
    if (aKeys.get() != NULL)
    {
        std::vector<CacheKey>::const_iterator iKey;
        std::vector<CacheKey>::const_iterator iEnd (aKeys->end());
        for (iKey=aKeys->begin(); iKey!=iEnd; ++iKey)
            rRequestQueue.AddRequest(*iKey, VISIBLE_NO_PREVIEW);
    }

    // Add the requests for the non-visible pages.
    aKeys = rpCacheContext->GetEntryList(false);
    if (aKeys.get() != NULL)
    {
        std::vector<CacheKey>::const_iterator iKey;
        std::vector<CacheKey>::const_iterator iEnd (aKeys->end());
        for (iKey=aKeys->begin(); iKey!=iEnd; ++iKey)
            rRequestQueue.AddRequest(*iKey, NOT_VISIBLE);
    }
}



} } } // end of namespace ::sd::slidesorter::cache



