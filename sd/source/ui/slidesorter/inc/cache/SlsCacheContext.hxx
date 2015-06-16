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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CACHE_SLSCACHECONTEXT_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CACHE_SLSCACHECONTEXT_HXX

#include <sal/types.h>
#include <com/sun/star/uno/XInterface.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

class SdrPage;
class Bitmap;

namespace sd { namespace slidesorter { namespace cache {

typedef const SdrPage* CacheKey;

/** This interface allows the individualisation of different instances of
    the PreviewCache.
*/
class CacheContext
{
public:
    virtual ~CacheContext() {}

    /** This method is called when the asynchronous creation of a preview
        has been finished.
        @param aKey
            The key of the page for which the preview has been created.
        @param aPreview
            The newly created preview.
    */
    virtual void NotifyPreviewCreation (
        CacheKey aKey,
        const Bitmap& rPreview) = 0;

    /** Called to determine whether the system is idle and a preview can be
        created without annoying the user.
    */
    virtual bool IsIdle() = 0;

    /** This method is used to determine whether a page is currently visible
        or not.  It is called when the cache becomes to large and some
        previews have to be released or scaled down.
    */
    virtual bool IsVisible (CacheKey aKey) = 0;

    /** Return the page associated with the given key.  Note that different
        keys may map to a single page (this may be the case with custom
        slide shows.)
    */
    virtual const SdrPage* GetPage (CacheKey aKey) = 0;

    /** This method is used when the request queue is filled.  It asks for
        the list of visible entries and maybe for the list of not visible
        entries and creates preview creation requests for them.
        @param bVisible
            When this is <FALSE/> then the implementation can decide whether
            to allow rendering of previews that are not visible (ahead of
            time). When not then return an empty pointer or an empty vector.
    */
    virtual ::boost::shared_ptr<std::vector<CacheKey> > GetEntryList (bool bVisible) = 0;

    /** Return the priority that defines the order in which previews are
        created for different keys/pages.  Typically the visible pages come
        first, then top-down, left-to-right.
    */
    virtual sal_Int32 GetPriority (CacheKey aKey) = 0;

    /** Return the model to which the pages belong for which the called
        cache manages the previews.  Different caches that belong to the
        same model but have different preview sizes may access previews of
        each other in order to create fast previews of the previews.
    */
    virtual ::com::sun::star::uno::Reference<com::sun::star::uno::XInterface> GetModel() = 0;
};

typedef ::boost::shared_ptr<CacheContext> SharedCacheContext;

} } } // end of namespace ::sd::slidesorter::cache

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
