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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CACHE_SLSPAGECACHE_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CACHE_SLSPAGECACHE_HXX

#include "cache/SlsCacheContext.hxx"
#include <sal/types.h>
#include <boost/scoped_ptr.hpp>
#include <vcl/bitmap.hxx>

class Size;

namespace sd { namespace slidesorter { namespace cache {

class GenericPageCache;

/** The page cache is responsible for the creation and storage of preview
    bitmaps of pages that are shown by the slide sorter.

    <p>Bitmaps for previews and a cache are used to speed up the display
    (painting) of the slide sorter.  But, of course, we have to limit this
    time-space-tradeoff by limiting the amount of space that can be use to
    store bitmaps.</p>

    <p>There are several strategies employed by this class to shorten the
    perceived time that is used to paint the slide sorter:
    <ul>
    <li>Rendering pages ahead of time.  Additionally to rendering the
    visible slides we try to render part or all of the slides that are not
    (yet) visible.  This, of course, makes sense only when the computer is
    ohterwise idle while doing that.</li>
    <li>When the size of the slides on the screen changes we mark the
    bitmaps as needing an update but use them while the new bitmap in the
    correct size is not available.</li>
    <li>Give the UI the chance to handle user events between the rendering
    of differe slides.</li>
    <li>Limit the amount of space that may be used for storing preview
    bitmaps and throw.</li>
    </p>

    <p>There is another somewhat similar methods for requesting new previews:
    GetPreviewBitmap() schedules a re-rendering (when necessary) and
    returns the preview what is currently available, either as a preview of
    the preview or, when nothing has changed since the last call, as the
    final thing.
    </p>
*/
class PageCache
{
public:
    /** The page chache is created with a reference to the slide sorter so
        that it has access to both the view and the model and so can fill
        itself with requests for all or just the visible pages.

        It is the task of the PageCacheManager to create new objects of this
        class.
    */
    PageCache (
        const Size& rPreviewSize,
        const bool bDoSuperSampling,
        const SharedCacheContext& rpCacheContext);

    ~PageCache();

    void ChangeSize(
        const Size& rPreviewSize,
        const bool bDoSuperSampling);

    /** Request a preview bitmap for the specified page object in the
        specified size.  The returned bitmap may be a preview of the
        preview, i.e. either a scaled (up or down) version of a previous
        preview (of the wrong size) or an empty bitmap.  In this case a
        request for the generation of a new preview is created and inserted
        into the request queue.  When the preview is available in the right
        size the page shape will be told to paint itself again.  When it
        then calls this method again if receives the correctly sized preview
        bitmap.
        @param rRequestData
            This data is used to determine the preview.
        @param bResize
            When <TRUE/> then when the available bitmap has not the
            requested size, it is scaled before it is returned.  When
            <FALSE/> then the bitmap is returned in the wrong size and it is
            the task of the caller to scale it.
        @return
            Returns a bitmap that is either empty, contains a scaled (up or
            down) version or is the requested bitmap.
    */
    Bitmap GetPreviewBitmap (
        const CacheKey aKey,
        const bool bResize);

    Bitmap GetMarkedPreviewBitmap (
        const CacheKey aKey,
        const bool bResize);
    void SetMarkedPreviewBitmap (
        const CacheKey aKey,
        const Bitmap& rBitmap);

    /** When the requested preview bitmap does not yet exist or is not
        up-to-date then the rendering of one is scheduled.  Otherwise this
        method does nothing.
    */
    void RequestPreviewBitmap (const CacheKey aKey);

    /** Tell the cache that the bitmap associated with the given request
        data is not up-to-date anymore.  This will invalidate all previews
        in other caches that represent the same page as well.
        @param bRequestPreview
            When <TRUE/> then a new preview is requested and will lead
            eventually to a repaint of the associated page object.
    */
    void InvalidatePreviewBitmap (
        const CacheKey aKey,
        const bool bRequestPreview);

    /** Call this method when all preview bitmaps have to be generated anew.
        This is the case when the size of the page objects on the screen has
        changed or when the model has changed.
        @param bUpdateCache
            When this flags is <TRUE/> then requests for updated previews
            are created.  When it is <FALSE/> the existing previews are only
            marked as not being up-to-date anymore.
    */
    void InvalidateCache (const bool bUpdateCache = true);

    /** With the precious flag you can control whether a bitmap can be
        removed or reduced in size to make room for other bitmaps or is so
        precious that it will not touched.  A typical use is to set the
        precious flag for exactly the visible pages.
    */
    void SetPreciousFlag (const CacheKey aKey, const bool bIsPrecious);

    void Pause();
    void Resume();

private:
    ::boost::scoped_ptr<GenericPageCache> mpImplementation;
};

} } } // end of namespace ::sd::slidesorter::cache

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
