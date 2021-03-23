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

#pragma once

#include <sal/types.h>
#include <vcl/timer.hxx>
#include <memory>

namespace sd::slidesorter::cache {

class BitmapCache;

/** This is an interface class whose implementations are created via the
    Create() factory method.
*/
class CacheCompactor
{
public:
    virtual ~CacheCompactor() {};

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
    static ::std::unique_ptr<CacheCompactor> Create (
        BitmapCache& rCache,
        sal_Int32 nMaximalCacheSize);

    /** Request a compaction of the off-screen previews in the bitmap
        cache.  This calls via a timer the Run() method.
    */
    virtual void RequestCompaction();

protected:
    BitmapCache& mrCache;
    sal_Int32 mnMaximalCacheSize;

    CacheCompactor(
        BitmapCache& rCache,
        sal_Int32 nMaximalCacheSize);

    /** This method actually tries to reduce the total number of bytes used
        by the off-screen preview bitmaps.
    */
    virtual void Run() = 0;

private:
    /** This timer is used to collect calls to RequestCompaction() and
        eventually call Run().
    */
    Timer maCompactionTimer;
    bool mbIsCompactionRunning;
    DECL_LINK(CompactionCallback, Timer *, void);
};

} // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
