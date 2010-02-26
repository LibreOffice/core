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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "lookupcache.hxx"
#include "document.hxx"

#ifdef erDEBUG
#include <cstdio>
using ::std::fprintf;
static long nCacheCount = 0;
#endif


ScLookupCache::ScLookupCache( ScDocument * pDoc, const ScRange & rRange ) :
    maRange( rRange),
    mpDoc( pDoc)
{
#ifdef erDEBUG
    ++nCacheCount;
    fprintf( stderr, "\nctor ScLookupCache %ld: %d, %d, %d, %d, %d, %d; buckets: %lu, size: %lu\n",
            nCacheCount,
            (int)getRange().aStart.Col(), (int)getRange().aStart.Row(),
            (int)getRange().aStart.Tab(), (int)getRange().aEnd.Col(),
            (int)getRange().aEnd.Row(), (int)getRange().aEnd.Tab(),
            (unsigned long)maQueryMap.bucket_count(), (unsigned long)maQueryMap.size());
#endif
}


ScLookupCache::~ScLookupCache()
{
#ifdef erDEBUG
    fprintf( stderr, "\ndtor ScLookupCache %ld: %d, %d, %d, %d, %d, %d; buckets: %lu, size: %lu\n",
            nCacheCount,
            (int)getRange().aStart.Col(), (int)getRange().aStart.Row(),
            (int)getRange().aStart.Tab(), (int)getRange().aEnd.Col(),
            (int)getRange().aEnd.Row(), (int)getRange().aEnd.Tab(),
            (unsigned long)maQueryMap.bucket_count(), (unsigned long)maQueryMap.size());
    --nCacheCount;
#endif
}


ScLookupCache::Result ScLookupCache::lookup( ScAddress & o_rResultAddress,
        const QueryCriteria & rCriteria, const ScAddress & rQueryAddress ) const
{
    QueryMap::const_iterator it( maQueryMap.find( QueryKey( rQueryAddress,
                    rCriteria.getQueryOp())));
    if (it == maQueryMap.end())
        return NOT_CACHED;
    const QueryCriteriaAndResult& rResult = (*it).second;
    if (!(rResult.maCriteria == rCriteria))
        return CRITERIA_DIFFERENT;
    if (rResult.maAddress.Row() < 0 )
        return NOT_AVAILABLE;
    o_rResultAddress = rResult.maAddress;
    return FOUND;
}


bool ScLookupCache::insert( const ScAddress & rResultAddress,
        const QueryCriteria & rCriteria, const ScAddress & rQueryAddress,
        const bool bAvailable )
{
#ifdef erDEBUG
    size_t nBuckets = maQueryMap.bucket_count();
#endif
    QueryKey aKey( rQueryAddress, rCriteria.getQueryOp());
    QueryCriteriaAndResult aResult( rCriteria, rResultAddress);
    if (!bAvailable)
        aResult.maAddress.SetRow(-1);
    bool bInserted = maQueryMap.insert( ::std::pair< const QueryKey,
            QueryCriteriaAndResult>( aKey, aResult)).second;
#ifdef erDEBUG
    if (nBuckets != maQueryMap.bucket_count())
    {
        fprintf( stderr, "\nbuck ScLookupCache: %d, %d, %d, %d, %d, %d; buckets: %lu, size: %lu\n",
                (int)getRange().aStart.Col(), (int)getRange().aStart.Row(),
                (int)getRange().aStart.Tab(), (int)getRange().aEnd.Col(),
                (int)getRange().aEnd.Row(), (int)getRange().aEnd.Tab(),
                (unsigned long)maQueryMap.bucket_count(), (unsigned long)maQueryMap.size());
    }
#endif
    return bInserted;
}


void ScLookupCache::Notify( SvtBroadcaster & /* rBC */ , const SfxHint &  rHint )
{
    if (!mpDoc->IsInDtorClear())
    {
        const ScHint* p = PTR_CAST( ScHint, &rHint );
        if (p && (p->GetId() & (SC_HINT_DATACHANGED | SC_HINT_DYING)))
        {
            mpDoc->RemoveLookupCache( *this);
            delete this;
        }
    }
}
