/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_FSTALGORITHM_HXX
#define INCLUDED_SC_INC_FSTALGORITHM_HXX

#include <mdds/flat_segment_tree.hpp>
#include <vector>

namespace sc {

template<typename Key, typename Span>
void buildSpan(
    std::vector<Span>& rSpans,
    typename mdds::flat_segment_tree<Key,bool>::const_iterator it,
    typename mdds::flat_segment_tree<Key,bool>::const_iterator itEnd, const Key* pStart )
{
    Key nLastPos = it->first;
    bool bLastVal = it->second;
    for (++it; it != itEnd; ++it)
    {
        Key nThisPos = it->first;
        bool bThisVal = it->second;

        if (bLastVal)
        {
            Key nIndex1 = nLastPos;
            Key nIndex2 = nThisPos-1;

            if (!pStart || *pStart < nIndex1)
                rSpans.push_back(Span(nIndex1, nIndex2));
            else if (*pStart <= nIndex2)
                rSpans.push_back(Span(*pStart, nIndex2));
        }

        nLastPos = nThisPos;
        bLastVal = bThisVal;
    }
}

template<typename Key, typename Val, typename Span>
void buildSpanWithValue(
    std::vector<Span>& rSpans,
    typename mdds::flat_segment_tree<Key,Val>::const_iterator it,
    typename mdds::flat_segment_tree<Key,Val>::const_iterator itEnd, const Key* pStart )
{
    Key nLastPos = it->first;
    Val nLastVal = it->second;
    for (++it; it != itEnd; ++it)
    {
        Key nThisPos = it->first;
        Val nThisVal = it->second;

        if (nLastVal)
        {
            Key nIndex1 = nLastPos;
            Key nIndex2 = nThisPos-1;

            if (!pStart || *pStart < nIndex1)
                rSpans.push_back(Span(nIndex1, nIndex2, nLastVal));
            else if (*pStart <= nIndex2)
                rSpans.push_back(Span(*pStart, nIndex2, nLastVal));
        }

        nLastPos = nThisPos;
        nLastVal = nThisVal;
    }
}

/**
 * Convert a flat_segment_tree structure whose value type is boolean, into
 * an array of ranges that corresponds with the segments that have a 'true'
 * value.
 */
template<typename Key, typename Span>
std::vector<Span> toSpanArray( const mdds::flat_segment_tree<Key,bool>& rTree )
{
    typedef mdds::flat_segment_tree<Key,bool> FstType;

    std::vector<Span> aSpans;

    typename FstType::const_iterator it = rTree.begin(), itEnd = rTree.end();
    buildSpan<Key,Span>(aSpans, it, itEnd, nullptr);
    return aSpans;
}

/**
 * Convert a flat_segment_tree structure into an array of ranges with
 * values.  Only those ranges whose value is evaluated to be true will be
 * included.  The value type must be something that supports bool operator.
 * The span type must support a constructor that takes a start key, an end
 * key and a value in this order.
 */
template<typename Key, typename Val, typename Span>
std::vector<Span> toSpanArrayWithValue( const mdds::flat_segment_tree<Key,Val>& rTree )
{
    typedef mdds::flat_segment_tree<Key,Val> FstType;

    std::vector<Span> aSpans;

    typename FstType::const_iterator it = rTree.begin(), itEnd = rTree.end();
    buildSpanWithValue<Key,Val,Span>(aSpans, it, itEnd, nullptr);
    return aSpans;
}

template<typename Key, typename Span>
std::vector<Span> toSpanArray( const mdds::flat_segment_tree<Key,bool>& rTree, Key nStartPos )
{
    typedef mdds::flat_segment_tree<Key,bool> FstType;

    std::vector<Span> aSpans;
    if (!rTree.is_tree_valid())
        return aSpans;

    bool bThisVal = false;
    std::pair<typename FstType::const_iterator, bool> r =
        rTree.search_tree(nStartPos, bThisVal);

    if (!r.second)
        // Tree search failed.
        return aSpans;

    typename FstType::const_iterator it = r.first, itEnd = rTree.end();
    buildSpan<Key,Span>(aSpans, it, itEnd, &nStartPos);
    return aSpans;
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
