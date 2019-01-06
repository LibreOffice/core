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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_CACHE_SLSGENERICPAGECACHE_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_CACHE_SLSGENERICPAGECACHE_HXX

#include "SlsRequestQueue.hxx"
#include <memory>

#include <vcl/bitmapex.hxx>

namespace sd { namespace slidesorter { namespace cache {

class BitmapCache;
class QueueProcessor;

/** This basically is the implementation class for the PageCache class.
*/
class GenericPageCache
{
public:
    /** The page cache is created with a reference to the SlideSorter and
        thus has access to both view and model.  This allows the cache to
        fill itself with requests for all pages or just the visible ones.
        @param rPreviewSize
            The size of the previews is expected in pixel values.
        @param bDoSuperSampling
            When <TRUE/> the previews are rendered larger and then scaled
            down to the requested size to improve image quality.
    */
    GenericPageCache (
        const Size& rPreviewSize,
        const bool bDoSuperSampling,
        const SharedCacheContext& rpCacheContext);

    ~GenericPageCache();

    /** Change the size of the preview bitmaps.  This may be caused by a
        resize of the slide sorter window or a change of the number of
        columns.
    */
    void ChangePreviewSize (
        const Size& rPreviewSize,
        const bool bDoSuperSampling);

    /** Request a preview bitmap for the specified page object in the
        specified size.  The returned bitmap may be a preview of the preview,
        i.e. either a scaled (up or down) version of a previous preview (of
        the wrong size) or an empty bitmap.  In this case a request for the
        generation of a new preview is created and inserted into the request
        queue.  When the preview is available the page shape will be told to
        paint itself again.  When it then calls this method again if
        receives the correctly sized preview bitmap.
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
    BitmapEx GetPreviewBitmap (
        const CacheKey aKey,
        const bool bResize);
    BitmapEx GetMarkedPreviewBitmap (
        const CacheKey aKey);
    void SetMarkedPreviewBitmap (
        const CacheKey aKey,
        const BitmapEx& rMarkedBitmap);

    /** When the requested preview bitmap does not yet exist or is not
        up-to-date then the rendering of one is scheduled.  Otherwise this
        method does nothing.
        @param rRequestData
            This data is used to determine the preview.
        @param bMayBeUpToDate
            This flag helps the method to determine whether an existing
            preview that matches the request is up to date.  If the caller
            knows that it is not then by passing <FALSE/> he tells us that we
            do not have to check the up-to-date flag a second time.  If
            unsure use <TRUE/>.
    */
    void RequestPreviewBitmap (
        const CacheKey aKey,
        const bool bMayBeUpToDate);

    /** Tell the cache to replace the bitmap associated with the given
        request data with a new one that reflects recent changes in the
        content of the page object.
        @return
            When the key is known then return <TRUE/>.
    */
    bool InvalidatePreviewBitmap (const CacheKey aKey);

    /** Call this method when all preview bitmaps have to be generated anew.
        This is the case when the size of the page objects on the screen has
        changed or when the model has changed.
    */
    void InvalidateCache ();

    /** With the precious flag you can control whether a bitmap can be
        removed from the cache or reduced in size to make room for other
        bitmaps or is so precious that it will not be touched.  A typical
        use is to set the precious flag for the visible pages.
    */
    void SetPreciousFlag (const CacheKey aKey, const bool bIsPrecious);

    void Pause();
    void Resume();

private:
    std::shared_ptr<BitmapCache> mpBitmapCache;

    RequestQueue maRequestQueue;

    std::unique_ptr<QueueProcessor> mpQueueProcessor;

    SharedCacheContext mpCacheContext;

    /** The current size of preview bitmaps.
    */
    Size maPreviewSize;

    bool mbDoSuperSampling;

    /** Both bitmap cache and queue processor are created on demand by this
        method.
    */
    void ProvideCacheAndProcessor();
};

} } } // end of namespace ::sd::slidesorter::cache

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
