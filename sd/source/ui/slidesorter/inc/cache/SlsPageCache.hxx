/*************************************************************************
 *
 *  $RCSfile: SlsPageCache.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:18:50 $
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

#ifndef SD_SLIDESORTER_PAGE_CACHE_HXX
#define SD_SLIDESORTER_PAGE_CACHE_HXX

#include <sal/types.h>
#include <vcl/bitmapex.hxx>
#include <memory>

namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
} } }

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
class PageObjectViewObjectContact;
} } }

namespace sd { namespace slidesorter { namespace cache {


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
    bitmaps and throw.  For this we need a policy for which bitmaps are to
    be disposed to make room for new ones.</li>
    </p>

    <p>There are three somewhat similar methods for requesting new previews:
    a) GetPreviewBitmap() schedules a re-rendering (when necessary) and
    returns the preview what is currently available, either as a preview of
    the preview or, when nothing has changed since the last call, as the
    final thing.
    b) RequestPreviewBitmap() schedules, like GetPreviewBitmap(), a
    re-rendering when the currently available preview is not up-to-date.  It
    does not, however, return anything.  Use this if you can wait for the
    preview.
    c) InvalidatePreviewBitmap() does not schedule a re-rendering, but
    remembers that one is necessary when one of the other two methods is
    called.
    </p>
    */
class PageCache
{
public:
    typedef view::PageObjectViewObjectContact RequestData;

    /** The page chache is created with references both to the view and the
        model so that it can fill itself with requests for all or just the
        visible pages.
    */
    PageCache (
        view::SlideSorterView& rView,
        model::SlideSorterModel& rModel,
        sal_Int32 nMaximalCacheSize);

    ~PageCache (void);

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
        RequestData& rRequestData,
        const Size& rSize);

    /** When the requested preview bitmap does not yet exist or is not
        up-to-date then the rendering of one is scheduled.  Otherwise this
        method does nothing.
    */
    void RequestPreviewBitmap (
        RequestData& rRequestData,
        const Size& rSize);

    /** Tell the cache that the bitmap associated with the given request
        data is not up-to-date anymore.  Unlike the RequestPreviewBitmap()
        method this does not trigger the rendering itself.  It just
        remembers to render it when the preview is requested the next time.
        @param rRequestData
            It is safe to pass a (barly) living object.  It will called only
            once to obtain its page object.
    */
    void InvalidatePreviewBitmap (const RequestData& rRequestData);

    /** Lower the priority with which the request associated with the given
        data will be processed.  Call this method when the visibility of a
        page object changes (from visible to not visible) and the request
        becomes a ahead-of-time request. When the request is already in the
        lowest class it will be removed.
    */
    void DecreaseRequestPriority (RequestData& rRequestData);

    /** Move the request associated with the given data into a higher
        priority class and increase its priority in that class above all
        other elements in the class.
    */
    void IncreaseRequestPriority (RequestData& rRequestData);

    /** Call this method when a view-object-contact object is being deleted
        and does not need (a) its current bitmap in the cache and (b) a
        requested new bitmap.
    */
    void ReleasePreviewBitmap (RequestData& rRequestData);

    /** Call this method when all preview bitmaps have to be generated anew.
        This is the case when the size of the page objects on the screen has
        changed or when the model has changed.
    */
    void InvalidateCache (void);

    /** With the precious flag you can control whether a bitmap can be
        removed or reduced in size to make room for other bitmaps or is so
        precious that it will not touched.  A typical use is to set the
        precious flag for exactly the visible pages.
    */
    void SetPreciousFlag (RequestData& rRequestData, bool bIsPrecious);

private:
    class PageCacheImplementation;
    ::std::auto_ptr<PageCacheImplementation> mpImplementation;
};

} } } // end of namespace ::sd::slidesorter::cache

#endif
