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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "lookupcache.hxx"
#include "document.hxx"

ScLookupCache::ScLookupCache( ScDocument * pDoc, const ScRange & rRange ) :
    maRange( rRange),
    mpDoc( pDoc)
{
}


ScLookupCache::~ScLookupCache()
{
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
    QueryKey aKey( rQueryAddress, rCriteria.getQueryOp());
    QueryCriteriaAndResult aResult( rCriteria, rResultAddress);
    if (!bAvailable)
        aResult.maAddress.SetRow(-1);
    bool bInserted = maQueryMap.insert( ::std::pair< const QueryKey,
            QueryCriteriaAndResult>( aKey, aResult)).second;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
