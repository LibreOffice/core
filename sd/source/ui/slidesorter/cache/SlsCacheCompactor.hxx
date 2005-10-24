/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsCacheCompactor.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-24 07:40:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
