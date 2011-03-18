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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
