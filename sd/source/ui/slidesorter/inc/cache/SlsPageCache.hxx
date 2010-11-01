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

#ifndef SD_SLIDESORTER_PAGE_CACHE_HXX
#define SD_SLIDESORTER_PAGE_CACHE_HXX

#include "cache/SlsCacheContext.hxx"
#include <sal/types.h>
#include <vcl/bitmapex.hxx>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace sd { namespace slidesorter { namespace view {
class PageObjectViewObjectContact;
} } }

namespace sd { namespace slidesorter { namespace cache {

class GenericPageCache;
class RequestData;

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
        const SharedCacheContext& rpCacheContext);

    ~PageCache (void);

    void ChangeSize(const Size& rPreviewSize);

    /** Request a preview bitmap for the specified page object in the
        specified size.  The returned bitmap may be a preview of the
        preview, i.e. either a scaled (up or down) version of a previous
        preview (of the wrong size) or an empty bitmap.  In this case a
        request for the generation of a new preview is created and inserted
        into the request queue.  When the preview is available the page
        shape will be told to paint itself again.  When it then calls this
        method again if receives the correctly sized preview bitmap.
        @param rRequestData
            This data is used to determine the preview.
        @param rSize
            The size of the requested preview bitmap.
        @return
            Returns a bitmap that is either empty, contains a scaled (up or
            down) version or is the requested bitmap.
    */
    BitmapEx GetPreviewBitmap (
        CacheKey aKey,
        const Size& rSize);

    /** Call this method when a view-object-contact object is being deleted
        and does not need (a) its current bitmap in the cache and (b) a
        requested new bitmap.
    */
    void ReleasePreviewBitmap (CacheKey aKey);

    /** Call this method when all preview bitmaps have to be generated anew.
        This is the case when the size of the page objects on the screen has
        changed or when the model has changed.
        @param bUpdateCache
            When this flags is <TRUE/> then requests for updated previews
            are created.  When it is <FALSE/> the existing previews are only
            marked as not being up-to-date anymore.
    */
    void InvalidateCache (bool bUpdateCache = true);

    /** With the precious flag you can control whether a bitmap can be
        removed or reduced in size to make room for other bitmaps or is so
        precious that it will not touched.  A typical use is to set the
        precious flag for exactly the visible pages.
    */
    void SetPreciousFlag (CacheKey aKey, bool bIsPrecious);

    void Pause (void);
    void Resume (void);

private:
    ::boost::scoped_ptr<GenericPageCache> mpImplementation;
};

} } } // end of namespace ::sd::slidesorter::cache

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
