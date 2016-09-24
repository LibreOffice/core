/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    std::shared_ptr<std::vector<CacheKey> > aKeys;

    // Add the requests for the visible pages.
    aKeys = rpCacheContext->GetEntryList(true);
    if (aKeys.get() != nullptr)
    {
        std::vector<CacheKey>::const_iterator iKey;
        std::vector<CacheKey>::const_iterator iEnd (aKeys->end());
        for (iKey=aKeys->begin(); iKey!=iEnd; ++iKey)
            rRequestQueue.AddRequest(*iKey, VISIBLE_NO_PREVIEW);
    }

    // Add the requests for the non-visible pages.
    aKeys = rpCacheContext->GetEntryList(false);
    if (aKeys.get() != nullptr)
    {
        std::vector<CacheKey>::const_iterator iKey;
        std::vector<CacheKey>::const_iterator iEnd (aKeys->end());
        for (iKey=aKeys->begin(); iKey!=iEnd; ++iKey)
            rRequestQueue.AddRequest(*iKey, NOT_VISIBLE);
    }
}

} } } // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
