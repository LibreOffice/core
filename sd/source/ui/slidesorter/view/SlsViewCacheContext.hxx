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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_VIEW_SLSVIEWCACHECONTEXT_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_VIEW_SLSVIEWCACHECONTEXT_HXX

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
    virtual ~ViewCacheContext();
    virtual void NotifyPreviewCreation (cache::CacheKey aKey, const Bitmap& rPreview) SAL_OVERRIDE;
    virtual bool IsIdle() SAL_OVERRIDE;
    virtual bool IsVisible (cache::CacheKey aKey) SAL_OVERRIDE;
    virtual const SdrPage* GetPage (cache::CacheKey aKey) SAL_OVERRIDE;
    virtual std::shared_ptr<std::vector<cache::CacheKey> > GetEntryList (bool bVisible) SAL_OVERRIDE;
    virtual sal_Int32 GetPriority (cache::CacheKey aKey) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference<com::sun::star::uno::XInterface> GetModel() SAL_OVERRIDE;

private:
    model::SlideSorterModel& mrModel;
    SlideSorter& mrSlideSorter;

    model::SharedPageDescriptor GetDescriptor (cache::CacheKey aKey);
};

} } } // end of namespace ::sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
