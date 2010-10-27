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

#ifndef SD_SLIDESORTER_GENERIC_PAGE_CACHE_HXX
#define SD_SLIDESORTER_GENERIC_PAGE_CACHE_HXX

#include "SlideSorter.hxx"
#include "SlsRequestQueue.hxx"
#include "SlsQueueProcessor.hxx"
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

namespace sd { namespace slidesorter { namespace cache {

class BitmapCache;
class QueueProcessor;

/** This basically is the implementation class for the PageCache class.
*/
class GenericPageCache
{
public:
    /** The page chache is created with references both to the SlideSorter.
        This allows access to both view and model and the cache can so fill
        itself with requests for all or just the visible pages.
    */
    GenericPageCache (
        const Size& rPreviewSize,
        const SharedCacheContext& rpCacheContext);

    ~GenericPageCache (void);

    /** Change the size of the preview bitmaps.  This may be caused by a
        resize of the slide sorter window or a change of the number of
        columns.
    */
    void ChangePreviewSize (const Size& rPreviewSize);

    /** Request a preview bitmap for the specified page object in the
        specified size.  The returned bitmap may be preview of the preview,
        i.e. either a scaled (up or down) version of a previous preview (of
        the wrong size) or an empty bitmap.  In this case a request for the
        generation of a new preview is created and inserted into the request
        queue.  When the preview is available the page shape will be told to
        paint itself again.  When it then calls this method again if
        receives the correctly sized preview bitmap.
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

    /** When the requested preview bitmap does not yet exist or is not
        up-to-date then the rendering of one is scheduled.  Otherwise this
        method does nothing.
        @param rRequestData
            This data is used to determine the preview.
        @param rSize
            The size of the requested preview bitmap in pixel coordinates.
        @param bMayBeUpToDate
            This flag helps the method to determine whether an existing
            preview that matches the request is up to date.  If the caller
            know that it is not then by passing <FALSE/> he tells us that we
            do not have to check the up-to-date flag a second time.  If
            unsure pass <TRUE/>.
    */
    void RequestPreviewBitmap (
        CacheKey aKey,
        const Size& rSize,
        bool bMayBeUpToDate = true);

    /** Call this method when a view-object-contact object is being deleted
        and does not need (a) its current bitmap in the cache and (b) a
        requested a new bitmap.
    */
    void ReleasePreviewBitmap (CacheKey aKey);

    /** Call this method when all preview bitmaps have to be generated anew.
        This is the case when the size of the page objects on the screen has
        changed or when the model has changed.
    */
    void InvalidateCache (bool bUpdateCache);

    /** With the precious flag you can control whether a bitmap can be
        removed from the cache or reduced in size to make room for other
        bitmaps or is so precious that it will not be touched.  A typical
        use is to set the precious flag for the visible pages.
    */
    void SetPreciousFlag (CacheKey aKey, bool bIsPrecious);

    void Pause (void);
    void Resume (void);

private:
    ::boost::shared_ptr<BitmapCache> mpBitmapCache;

    RequestQueue maRequestQueue;

    ::boost::scoped_ptr<QueueProcessor> mpQueueProcessor;

    SharedCacheContext mpCacheContext;

    /** The current size of preview bitmaps.
    */
    Size maPreviewSize;

    /** Both bitmap cache and queue processor are created on demand by this
        method.
    */
    void ProvideCacheAndProcessor (void);
};


} } } // end of namespace ::sd::slidesorter::cache

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
