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

#include <lookupcache.hxx>
#include <document.hxx>
#include <queryentry.hxx>
#include <brdcst.hxx>

#include <sal/log.hxx>

ScLookupCache::QueryCriteria::QueryCriteria( const ScQueryEntry& rEntry ) :
    mfVal(0.0), mbAlloc(false), mbString(false)
{
    switch (rEntry.eOp)
    {
        case SC_EQUAL :
            meOp = EQUAL;
            break;
        case SC_LESS_EQUAL :
            meOp = LESS_EQUAL;
            break;
        case SC_GREATER_EQUAL :
            meOp = GREATER_EQUAL;
            break;
        default:
            meOp = UNKNOWN;
            SAL_WARN( "sc.core", "ScLookupCache::QueryCriteria not prepared for this ScQueryOp");
    }

    const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
    if (rItem.meType == ScQueryEntry::ByString)
        setString(rItem.maString.getString());
    else
        setDouble(rItem.mfVal);
}

ScLookupCache::QueryCriteria::QueryCriteria( const ScLookupCache::QueryCriteria & r ) :
    mfVal( r.mfVal),
    mbAlloc( false),
    mbString( false),
    meOp( r.meOp)
{
    if (r.mbString && r.mpStr)
    {
        mpStr = new OUString( *r.mpStr);
        mbAlloc = mbString = true;
    }
}

ScLookupCache::QueryCriteria::~QueryCriteria()
{
    deleteString();
}

ScLookupCache::ScLookupCache( ScDocument * pDoc, const ScRange & rRange, ScLookupCacheMap & cacheMap ) :
    maRange( rRange),
    mpDoc( pDoc),
    mCacheMap(cacheMap)
{
}

ScLookupCache::~ScLookupCache()
{
}

ScLookupCache::Result ScLookupCache::lookup( ScAddress & o_rResultAddress,
        const QueryCriteria & rCriteria, const ScAddress & rQueryAddress ) const
{
    auto it( maQueryMap.find( QueryKey( rQueryAddress,
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

SCROW ScLookupCache::lookup( const QueryCriteria & rCriteria ) const
{
    // try to find the row index for which we have already performed lookup
    auto it = std::find_if(maQueryMap.begin(), maQueryMap.end(),
        [&rCriteria](const std::pair<QueryKey, QueryCriteriaAndResult>& rEntry) {
            return rEntry.second.maCriteria == rCriteria;
        });
    if (it != maQueryMap.end())
        return it->first.mnRow;

    // not found
    return -1;
}

bool ScLookupCache::insert( const ScAddress & rResultAddress,
        const QueryCriteria & rCriteria, const ScAddress & rQueryAddress,
        const bool bAvailable )
{
    QueryKey aKey( rQueryAddress, rCriteria.getQueryOp());
    QueryCriteriaAndResult aResult( rCriteria, rResultAddress);
    if (!bAvailable)
        aResult.maAddress.SetRow(-1);
    bool bInserted = maQueryMap.insert( ::std::pair< const QueryKey,
            QueryCriteriaAndResult>( aKey, aResult)).second;

    return bInserted;
}

void ScLookupCache::Notify( const SfxHint& rHint )
{
    if (!mpDoc->IsInDtorClear())
    {
        const ScHint* p = dynamic_cast<const ScHint*>(&rHint);
        if ((p && (p->GetId() == SfxHintId::ScDataChanged)) || dynamic_cast<const ScAreaChangedHint*>(&rHint))
        {
            mpDoc->RemoveLookupCache( *this);
            delete this;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
