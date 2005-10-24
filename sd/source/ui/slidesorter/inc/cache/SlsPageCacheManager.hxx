/*************************************************************************
 *
 *  $RCSfile: SlsPageCacheManager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-24 07:43:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SD_PAGE_CACHE_MANAGER_HXX
#define SD_PAGE_CACHE_MANAGER_HXX

#include <sal/types.h>
#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

class Size;
class SdDrawDocument;
class SdrPage;

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
class PageObjectViewObjectContact;
} } }

namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
} } }

namespace sd { namespace slidesorter { namespace cache {

class BitmapCache;


/** Provide and manage the preview bitmap caches for all slide sorter
    instances.  There is one cache per active slide sorter plus a small
    number of caches that are no longer in use.  The later are kept to speed
    up the switching between views.
*/
class PageCacheManager
{
public:
    typedef BitmapCache Cache;
    typedef ::std::vector< ::std::pair<Size, ::boost::shared_ptr<BitmapCache> > > BestFittingPageCaches;

    /** Return the one instance of the PageCacheManager class.
    */
    static ::boost::shared_ptr<PageCacheManager> Instance (void);

    /** Look up the cache for the given model in which the previews have the
        specified size.  If no such cache exists, then one is created.  When
        a new BitmapCache is created its Recycle() method is called with a
        sorted list of existing caches from which the new one initialize its
        previews.
        @return
            The returned cache lives as long as somebody keeps a shared
            pointer and the ReleaseCache() method has not been called.
    */
    ::boost::shared_ptr<Cache> GetCache (
        SdDrawDocument* pDocument,
        const Size& rPreviewSize);

    /** Tell the cache manager to release its own reference to the specified
        cache.  After that the cache will live as long as the caller (and
        maybe others) holds its reference.
    */
    void ReleaseCache (const ::boost::shared_ptr<Cache>& rpCache);

    /** This is an information to the cache manager that the size of preview
        bitmaps in the specified cache has changed.

    */
    ::boost::shared_ptr<Cache> ChangeSize (
        const ::boost::shared_ptr<Cache>& rpCache,
        const Size& rOldPreviewSize,
        const Size& rNewPreviewSize);

    /** Invalidate the preview bitmap for one slide that belongs to the
        specified document.  The bitmaps for this slide in all caches are
        marked as out-of-date and will be re-created when they are requested
        the next time.
    */
    void InvalidatePreviewBitmap (
        SdDrawDocument* pDocument,
        const SdrPage* pPage);

    /** The destructor, like the constructor is, should be private.  It can
        not because the shared_ptr<> implementation does not allow this.
    */
    ~PageCacheManager (void);

private:
    /// Singleton instance of the cache manager.
    static ::boost::shared_ptr<PageCacheManager> mpInstance;

    /// List of active caches.
    class PageCacheContainer;
    ::std::auto_ptr<PageCacheContainer> mpPageCaches;

    /// List of inactive, recently used caches.
    class RecentlyUsedPageCaches;
    ::std::auto_ptr<RecentlyUsedPageCaches> mpRecentlyUsedPageCaches;

    /** The maximal number of recently used caches that are kept alive after
        they have become inactive, i.e. after they are not used anymore by a
        slide sorter.
    */
    const sal_uInt32 mnMaximalRecentlyCacheCount;

    PageCacheManager (void);

    ::boost::shared_ptr<Cache> GetRecentlyUsedCache(
        SdDrawDocument* pDocument,
        const Size& rSize);

    /** Add the given cache to the list of recently used caches for the
        document.  There is one such list per document.  Each least has at
        most mnMaximalRecentlyCacheCount members.
    */
    void PutRecentlyUsedCache(
        SdDrawDocument* pDocument,
        const Size& rPreviewSize,
        const ::boost::shared_ptr<Cache>& rpCache);

    /** Return a sorted list of the available caches, both active caches and
        those recently used, for the given document.  The sort order is so
        that an exact match of the preview size is at the front.  Other
        caches follow with the largest size first.
    */
    BestFittingPageCaches GetBestFittingCaches (
        SdDrawDocument* pDocument,
        const Size& rPreviewSize);

    /** This method is used internally to initialize a newly created
        BitmapCache with already exisiting previews.
    */
    void Recycle (
        const ::boost::shared_ptr<Cache>& rpCache,
        SdDrawDocument* pDocument,
        const Size& rPreviewSize);
};

} } } // end of namespace ::sd::slidesorter::cache

#endif
