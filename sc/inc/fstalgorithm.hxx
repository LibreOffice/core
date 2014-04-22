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

template<typename _Key, typename _Span>
void buildSpan(
    std::vector<_Span>& rSpans,
    typename mdds::flat_segment_tree<_Key,bool>::const_iterator it,
    typename mdds::flat_segment_tree<_Key,bool>::const_iterator itEnd, const _Key* pStart )
{
    _Key nLastPos = it->first;
    bool bLastVal = it->second;
    for (++it; it != itEnd; ++it)
    {
        _Key nThisPos = it->first;
        bool bThisVal = it->second;

        if (bLastVal)
        {
            _Key nIndex1 = nLastPos;
            _Key nIndex2 = nThisPos-1;

            if (!pStart || *pStart < nIndex1)
                rSpans.push_back(_Span(nIndex1, nIndex2));
            else if (*pStart <= nIndex2)
                rSpans.push_back(_Span(*pStart, nIndex2));
        }

        nLastPos = nThisPos;
        bLastVal = bThisVal;
    }
}

/**
 * Convert a flat_segment_tree structure whose value type is boolean, into
 * an array of ranges that corresponds with the segments that have a 'true'
 * value.
 */
template<typename _Key, typename _Span>
std::vector<_Span> toSpanArray( const mdds::flat_segment_tree<_Key,bool>& rTree )
{
    typedef mdds::flat_segment_tree<_Key,bool> FstType;

    std::vector<_Span> aSpans;

    typename FstType::const_iterator it = rTree.begin(), itEnd = rTree.end();
    buildSpan<_Key,_Span>(aSpans, it, itEnd, NULL);
    return aSpans;
}

template<typename _Key, typename _Span>
std::vector<_Span> toSpanArray( const mdds::flat_segment_tree<_Key,bool>& rTree, _Key nStartPos )
{
    typedef mdds::flat_segment_tree<_Key,bool> FstType;

    std::vector<_Span> aSpans;
    if (!rTree.is_tree_valid())
        return aSpans;

    bool bThisVal = false;
    std::pair<typename FstType::const_iterator, bool> r =
        rTree.search_tree(nStartPos, bThisVal);

    if (!r.second)
        // Tree search failed.
        return aSpans;

    typename FstType::const_iterator it = r.first, itEnd = rTree.end();
    buildSpan<_Key,_Span>(aSpans, it, itEnd, &nStartPos);
    return aSpans;
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
