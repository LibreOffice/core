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

#ifndef SD_SLIDESORTER_CACHE_COMPACTOR_HXX
#define SD_SLIDESORTER_CACHE_COMPACTOR_HXX

#include <sal/types.h>
#include <vcl/timer.hxx>
#include <memory>

namespace sd { namespace slidesorter { namespace cache {

class BitmapCache;
class BitmapCompressor;

/** This is an interface class whose implementations are created via the
    Create() factory method.
*/
class CacheCompactor
{
public:
    virtual ~CacheCompactor (void) {};

    /** Create a new instance of the CacheCompactor interface class.  The
        type of compaction algorithm used depends on values from the
        configuration: the SlideSorter/PreviewCache/CompactionPolicy
        property of the Impress.xcs file currently supports the values
        "None" and "Compress".  With the later the CompressionPolicy
        property is evaluated which implementation of the BitmapCompress
        interface class to use as bitmap compressor.
        @param rCache
            The bitmap cache on which to operate.
        @param nMaximalCacheSize
            The total number of bytes the off-screen bitmaps in the cache
            may have.  When the Run() method is (indirectly) called the
            compactor tries to reduce that summed size of off-screen bitmaps
            under this number.  However, it is not guaranteed that this
            works in all cases.
    */
    static ::std::auto_ptr<CacheCompactor> Create (
        BitmapCache& rCache,
        sal_Int32 nMaximalCacheSize);

    /** Request a compaction of the off-screen previews in the bitmap
        cache.  This calls via a timer the Run() method.
    */
    virtual void RequestCompaction (void);

protected:
    BitmapCache& mrCache;
    sal_Int32 mnMaximalCacheSize;

    CacheCompactor(
        BitmapCache& rCache,
        sal_Int32 nMaximalCacheSize);

    /** This method actually tries to reduce the total number of bytes used
        by the off-screen preview bitmaps.
    */
    virtual void Run (void) = 0;

private:
    /** This timer is used to collect calles to RequestCompaction() and
        eventually call Run().
    */
    Timer maCompactionTimer;
    bool mbIsCompactionRunning;
    DECL_LINK(CompactionCallback, Timer*);
};




} } } // end of namespace ::sd::slidesorter::cache

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
