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

#include "address.hxx"
#include <svl/listener.hxx>

#include <memory>
#include <unordered_map>

class ScDocument;
struct ScLookupCacheMap;
struct ScQueryEntry;

/** Lookup cache for one range used with interpreter functions such as VLOOKUP
    and MATCH. Caches query for a specific row and the resulting address looked
    up, in case other lookups of the same query in the same row are to be
    performed, which usually occur to obtain a different offset column of the
    same query.
 */

class ScLookupCache final : public SvtListener
{
public:

    enum Result
    {
        NOT_CACHED,         /// Query not found in cache.
        CRITERIA_DIFFERENT, /// Different criteria for same query position exists.
        NOT_AVAILABLE,      /// Criteria not available in lookup range.
        FOUND               /// Criteria found.
    };

    enum QueryOp
    {
        UNKNOWN,
        EQUAL,
        LESS_EQUAL,
        GREATER_EQUAL
    };

    class QueryCriteria
    {
        union
        {
            double          mfVal;
            const OUString *mpStr;
        };
        bool                mbAlloc;
        bool                mbString;
        QueryOp             meOp;

        void deleteString()
        {
            if (mbAlloc && mbString)
                delete mpStr;
        }

        QueryCriteria & operator=( const QueryCriteria & r ) = delete;

    public:

        explicit QueryCriteria( const ScQueryEntry & rEntry );
        QueryCriteria( const QueryCriteria & r );
        ~QueryCriteria();

        QueryOp getQueryOp() const { return meOp; }

        void setDouble( double fVal )
        {
            deleteString();
            mbAlloc = mbString = false;
            mfVal = fVal;
        }

        void setString( const OUString & rStr )
        {
            deleteString();
            mbAlloc = mbString = true;
            mpStr = new OUString( rStr);
        }

        bool operator==( const QueryCriteria & r ) const
        {
            return meOp == r.meOp && mbString == r.mbString &&
                (mbString ? (*mpStr == *r.mpStr) : (mfVal == r.mfVal));
        }

        bool isEmptyStringQuery() const
        {
            return (getQueryOp() == QueryOp::EQUAL) && mbString && mpStr && mpStr->isEmpty();
        }
    };

    /// MUST be new'd because Notify() deletes.
                            ScLookupCache( ScDocument * pDoc, const ScRange & rRange, ScLookupCacheMap & cacheMap );
    virtual                 ~ScLookupCache() override;
    /// Remove from document structure and delete (!) cache on modify hint.
    virtual void Notify( const SfxHint& rHint ) override;

    /// @returns document address in o_rResultAddress if Result==FOUND
            Result          lookup( ScAddress & o_rResultAddress,
                                    const QueryCriteria & rCriteria,
                                    const ScAddress & rQueryAddress ) const;

            SCROW           lookup( const QueryCriteria & rCriteria ) const;

    /** Insert query and result.
        @param bAvailable
            Pass sal_False if the search didn't deliver a result. A subsequent
            lookup() then will return Result::NOT_AVAILABLE.
        @returns successful insertion.
      */
            bool            insert( const ScAddress & rResultAddress,
                                    const QueryCriteria & rCriteria,
                                    const ScAddress & rQueryAddress,
                                    const bool bAvailable );

    const ScRange&  getRange() const { return maRange; }

    ScLookupCacheMap & getCacheMap() const { return mCacheMap; }

    struct Hash
    {
        size_t operator()( const ScRange & rRange ) const
        {
            // Lookups are performed on the first column.
            return rRange.hashStartColumn();
        }
    };

private:

    struct QueryKey
    {
        SCROW           mnRow;
        SCTAB           mnTab;
        QueryOp         meOp;

        QueryKey( const ScAddress & rAddress, const QueryOp eOp ) :
            mnRow( rAddress.Row()),
            mnTab( rAddress.Tab()),
            meOp( eOp)
        {
        }

        bool operator==( const QueryKey & r ) const
        {
            return mnRow == r.mnRow && mnTab == r.mnTab && meOp == r.meOp && meOp != UNKNOWN;
        }

        struct Hash
        {
            size_t operator()( const QueryKey & r ) const
            {
                return (static_cast<size_t>(r.mnTab) << 24) ^
                    (static_cast<size_t>(r.meOp) << 22) ^
                    static_cast<size_t>(r.mnRow);
            }
        };
    };

    struct QueryCriteriaAndResult
    {
        QueryCriteria   maCriteria;
        ScAddress       maAddress;

        QueryCriteriaAndResult( const QueryCriteria & rCriteria, const ScAddress & rAddress ) :
            maCriteria( rCriteria),
            maAddress( rAddress)
        {
        }
    };

    std::unordered_map< QueryKey, QueryCriteriaAndResult, QueryKey::Hash > maQueryMap;
    ScRange         maRange;
    ScDocument *    mpDoc;
    ScLookupCacheMap & mCacheMap;

    ScLookupCache( const ScLookupCache & ) = delete;
    ScLookupCache & operator=( const ScLookupCache & ) = delete;

};

// Struct because including lookupcache.hxx in document.hxx isn't wanted.
struct ScLookupCacheMap
{
    std::unordered_map< ScRange, std::unique_ptr<ScLookupCache>, ScLookupCache::Hash > aCacheMap;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
