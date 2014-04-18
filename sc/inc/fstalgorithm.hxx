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
    _Key nLastPos = it->first;
    bool bLastVal = it->second;
    for (++it; it != itEnd; ++it)
    {
        _Key nThisPos = it->first;
        bool bThisVal = it->second;

        if (bLastVal)
            aSpans.push_back(_Span(nLastPos, nThisPos-1));

        nLastPos = nThisPos;
        bLastVal = bThisVal;
    }

    return aSpans;
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
