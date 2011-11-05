/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2011 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef __SC_QUERYENTRY_HXX__
#define __SC_QUERYENTRY_HXX__

#include "address.hxx"
#include "global.hxx"

#include <vector>

namespace utl {
    class SearchParam;
    class TextSearch;
}

/**
 * Each instance of this struct represents a single filtering criteria.
 */
struct ScQueryEntry
{
    typedef std::vector<rtl::OUString> QueryStringsType;

    bool            bDoQuery;
    bool            bQueryByString;
    bool            bQueryByDate;
    SCCOLROW        nField;
    ScQueryOp       eOp;
    ScQueryConnect  eConnect;
    double          nVal;
    mutable utl::SearchParam* pSearchParam;       // if RegExp, not saved
    mutable utl::TextSearch*  pSearchText;        // if RegExp, not saved

    ScQueryEntry();
    ScQueryEntry(const ScQueryEntry& r);
    ~ScQueryEntry();

    // creates pSearchParam and pSearchText if necessary, always RegExp!
    utl::TextSearch* GetSearchTextPtr( bool bCaseSens ) const;

    bool            IsQueryStringEmpty() const;
    bool            MatchByString(const rtl::OUString& rStr, bool bCaseSens) const;
    void            SwapQueryStrings(QueryStringsType& rStrings);
    void            SortQueryStrings(bool bCaseSens);
    SC_DLLPUBLIC void SetQueryString(const rtl::OUString& rStr);
    SC_DLLPUBLIC rtl::OUString GetQueryString() const;
    void            Clear();
    ScQueryEntry&   operator=( const ScQueryEntry& r );
    bool            operator==( const ScQueryEntry& r ) const;
private:
    QueryStringsType maQueryStrings;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
