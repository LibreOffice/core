/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        const ScHint* p = dynamic_cast< const ScHint* >( &rHint );
        if (p && (p->GetId() & (SC_HINT_DATACHANGED | SC_HINT_DYING)))
        {
            mpDoc->RemoveLookupCache( *this);
            delete this;
        }
    }
}
