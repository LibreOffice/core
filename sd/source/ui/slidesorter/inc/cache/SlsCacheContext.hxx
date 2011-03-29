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

#ifndef SD_SLIDESORTER_CACHE_CONTEXT_HXX
#define SD_SLIDESORTER_CACHE_CONTEXT_HXX

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
    virtual bool IsIdle (void) = 0;

    /** This method is used to determine whether a page is currently visible
        or not.  It is called when the cache becomes to large and some
        previews have to be released or scaled down.
    */
    virtual bool IsVisible (CacheKey aKey) = 0;

    /** Return the page associdated with the given key.  Note that different
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
        same model but have different preview sizes may acces previews of
        each other in order to create fast previews of the previews.
    */
    virtual ::com::sun::star::uno::Reference<com::sun::star::uno::XInterface> GetModel (void) = 0;
};

typedef ::boost::shared_ptr<CacheContext> SharedCacheContext;

} } } // end of namespace ::sd::slidesorter::cache

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
