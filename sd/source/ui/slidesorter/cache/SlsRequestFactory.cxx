/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsRequestFactory.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:20:52 $
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

#include "SlsRequestFactory.hxx"

#include "SlsGenericPageCache.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "view/SlideSorterView.hxx"
#include <svx/svdpagv.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdr/contact/viewcontact.hxx>


namespace sd { namespace slidesorter { namespace cache {

void RequestFactory::operator()(
    RequestQueue& rRequestQueue,
    const SharedCacheContext& rpCacheContext)
{
    ::boost::shared_ptr<std::vector<CacheKey> > aKeys;

    // Add the requests for the visible pages.
    aKeys = rpCacheContext->GetEntryList(true);
    if (aKeys.get() != NULL)
    {
        std::vector<CacheKey>::const_iterator iKey;
        std::vector<CacheKey>::const_iterator iEnd (aKeys->end());
        for (iKey=aKeys->begin(); iKey!=iEnd; ++iKey)
            rRequestQueue.AddRequest(*iKey, VISIBLE_NO_PREVIEW);
    }

    // Add the requests for the non-visible pages.
    aKeys = rpCacheContext->GetEntryList(false);
    if (aKeys.get() != NULL)
    {
        std::vector<CacheKey>::const_iterator iKey;
        std::vector<CacheKey>::const_iterator iEnd (aKeys->end());
        for (iKey=aKeys->begin(); iKey!=iEnd; ++iKey)
            rRequestQueue.AddRequest(*iKey, NOT_VISIBLE);
    }
}



} } } // end of namespace ::sd::slidesorter::cache
