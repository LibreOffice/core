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

#include "queryentry.hxx"

#include <unotools/textsearch.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/collatorwrapper.hxx>

ScQueryEntry::ScQueryEntry() :
    bDoQuery(false),
    bQueryByString(false),
    bQueryByDate(false),
    nField(0),
    eOp(SC_EQUAL),
    eConnect(SC_AND),
    nVal(0.0),
    pSearchParam(NULL),
    pSearchText(NULL)
{
}

ScQueryEntry::ScQueryEntry(const ScQueryEntry& r) :
    bDoQuery(r.bDoQuery),
    bQueryByString(r.bQueryByString),
    bQueryByDate(r.bQueryByDate),
    nField(r.nField),
    eOp(r.eOp),
    eConnect(r.eConnect),
    nVal(r.nVal),
    pSearchParam(NULL),
    pSearchText(NULL),
    maQueryStrings(r.maQueryStrings)
{
}

ScQueryEntry::~ScQueryEntry()
{
    delete pSearchParam;
    delete pSearchText;
}

ScQueryEntry& ScQueryEntry::operator=( const ScQueryEntry& r )
{
    bDoQuery        = r.bDoQuery;
    bQueryByString  = r.bQueryByString;
    bQueryByDate    = r.bQueryByDate;
    eOp             = r.eOp;
    eConnect        = r.eConnect;
    nField          = r.nField;
    nVal            = r.nVal;
    maQueryStrings  = r.maQueryStrings;

    delete pSearchParam;
    delete pSearchText;
    pSearchParam    = NULL;
    pSearchText     = NULL;

    return *this;
}

bool ScQueryEntry::IsQueryStringEmpty() const
{
    return maQueryStrings.empty();
}

namespace {

class CompareString : std::binary_function<rtl::OUString, rtl::OUString, bool>
{
    CollatorWrapper* mpCollator;
public:
    CompareString(bool bCaseSens) :
     mpCollator(
         bCaseSens ? ScGlobal::GetCaseCollator() : ScGlobal::GetCollator())
    {}

    bool operator() (const rtl::OUString& rL, const rtl::OUString& rR) const
    {
        return mpCollator->compareString(rL, rR) < 0;
    }
};

}

bool ScQueryEntry::MatchByString(const rtl::OUString& rStr, bool bCaseSens) const
{
    QueryStringsType::const_iterator itr =
        std::lower_bound(
            maQueryStrings.begin(), maQueryStrings.end(), rStr, CompareString(bCaseSens));

    if (itr == maQueryStrings.end())
        return false;

    utl::TransliterationWrapper* pTransliteration =
        bCaseSens ? ScGlobal::GetCaseTransliteration() : ScGlobal::GetpTransliteration();
    return pTransliteration->isEqual(rStr, *itr);
}

void ScQueryEntry::SwapQueryStrings(QueryStringsType& rStrings)
{
    maQueryStrings.swap(rStrings);
}

void ScQueryEntry::SortQueryStrings(bool bCaseSens)
{
    std::sort(maQueryStrings.begin(), maQueryStrings.end(), CompareString(bCaseSens));
}

void ScQueryEntry::SetQueryString(const rtl::OUString& rStr)
{
    maQueryStrings.clear();
    if (!rStr.isEmpty())
        maQueryStrings.push_back(rStr);
}

rtl::OUString ScQueryEntry::GetQueryString() const
{
    return maQueryStrings.empty() ? rtl::OUString() : maQueryStrings[0];
}

void ScQueryEntry::Clear()
{
    bDoQuery        = false;
    bQueryByString  = false;
    bQueryByDate    = false;
    eOp             = SC_EQUAL;
    eConnect        = SC_AND;
    nField          = 0;
    nVal            = 0.0;
    maQueryStrings.clear();

    delete pSearchParam;
    delete pSearchText;
    pSearchParam    = NULL;
    pSearchText     = NULL;
}

bool ScQueryEntry::operator==( const ScQueryEntry& r ) const
{
    return bDoQuery         == r.bDoQuery
        && bQueryByString   == r.bQueryByString
        && bQueryByDate     == r.bQueryByDate
        && eOp              == r.eOp
        && eConnect         == r.eConnect
        && nField           == r.nField
        && nVal             == r.nVal
        && maQueryStrings   == r.maQueryStrings;
    //! pSearchParam und pSearchText nicht vergleichen
}

utl::TextSearch* ScQueryEntry::GetSearchTextPtr( bool bCaseSens ) const
{
    if ( !pSearchParam )
    {
        rtl::OUString aStr;
        if (!maQueryStrings.empty())
            aStr = maQueryStrings[0];
        pSearchParam = new utl::SearchParam(
            aStr, utl::SearchParam::SRCH_REGEXP, bCaseSens, false, false);
        pSearchText = new utl::TextSearch( *pSearchParam, *ScGlobal::pCharClass );
    }
    return pSearchText;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
