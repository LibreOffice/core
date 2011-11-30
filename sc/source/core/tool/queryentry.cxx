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

/*
 * dialog returns the special field values "empty"/"not empty"
 * as constants SC_EMPTYFIELDS and SC_NONEMPTYFIELDS respectively in nVal in
 * conjuctions with the flag bQueryByString = FALSE.
 */

#define SC_EMPTYFIELDS      ((double)0x0042)
#define SC_NONEMPTYFIELDS   ((double)0x0043)

bool ScQueryEntry::Item::operator== (const Item& r) const
{
    return meType == r.meType && mfVal == r.mfVal && maString.equals(r.maString);
}

ScQueryEntry::ScQueryEntry() :
    bDoQuery(false),
    nField(0),
    eOp(SC_EQUAL),
    eConnect(SC_AND),
    pSearchParam(NULL),
    pSearchText(NULL),
    maQueryItems(1)
{
}

ScQueryEntry::ScQueryEntry(const ScQueryEntry& r) :
    bDoQuery(r.bDoQuery),
    nField(r.nField),
    eOp(r.eOp),
    eConnect(r.eConnect),
    pSearchParam(NULL),
    pSearchText(NULL),
    maQueryItems(r.maQueryItems)
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
    eOp             = r.eOp;
    eConnect        = r.eConnect;
    nField          = r.nField;
    maQueryItems  = r.maQueryItems;

    delete pSearchParam;
    delete pSearchText;
    pSearchParam    = NULL;
    pSearchText     = NULL;

    return *this;
}

ScQueryEntry::QueryItemsType& ScQueryEntry::GetQueryItems()
{
    return maQueryItems;
}

const ScQueryEntry::QueryItemsType& ScQueryEntry::GetQueryItems() const
{
    return maQueryItems;
}

void ScQueryEntry::SetQueryByEmpty()
{
    eOp = SC_EQUAL;
    maQueryItems.resize(1);
    Item& rItem = maQueryItems[0];
    rItem.meType = ByEmpty;
    rItem.maString = rtl::OUString();
    rItem.mfVal = SC_EMPTYFIELDS;
}

bool ScQueryEntry::IsQueryByEmpty() const
{
    if (maQueryItems.size() != 1)
        return false;

    const Item& rItem = maQueryItems[0];
    return eOp == SC_EQUAL &&
        rItem.meType == ByEmpty &&
        rItem.maString.isEmpty() &&
        rItem.mfVal == SC_EMPTYFIELDS;
}

void ScQueryEntry::SetQueryByNonEmpty()
{
    eOp = SC_EQUAL;
    maQueryItems.resize(1);
    Item& rItem = maQueryItems[0];
    rItem.meType = ByEmpty;
    rItem.maString = rtl::OUString();
    rItem.mfVal = SC_NONEMPTYFIELDS;
}

bool ScQueryEntry::IsQueryByNonEmpty() const
{
    if (maQueryItems.size() != 1)
        return false;

    const Item& rItem = maQueryItems[0];
    return eOp == SC_EQUAL &&
        rItem.meType == ByEmpty &&
        rItem.maString.isEmpty() &&
        rItem.mfVal == SC_NONEMPTYFIELDS;
}

const ScQueryEntry::Item& ScQueryEntry::GetQueryItem() const
{
    if (maQueryItems.size() > 1)
        // Reset to a single query mode.
        maQueryItems.resize(1);
    return maQueryItems[0];
}

ScQueryEntry::Item& ScQueryEntry::GetQueryItem()
{
    if (maQueryItems.size() > 1)
        // Reset to a single query mode.
        maQueryItems.resize(1);
    return maQueryItems[0];
}

void ScQueryEntry::Clear()
{
    bDoQuery        = false;
    eOp             = SC_EQUAL;
    eConnect        = SC_AND;
    nField          = 0;
    maQueryItems.clear();
    maQueryItems.push_back(Item());

    delete pSearchParam;
    delete pSearchText;
    pSearchParam    = NULL;
    pSearchText     = NULL;
}

bool ScQueryEntry::operator==( const ScQueryEntry& r ) const
{
    return bDoQuery         == r.bDoQuery
        && eOp              == r.eOp
        && eConnect         == r.eConnect
        && nField           == r.nField
        && maQueryItems   == r.maQueryItems;
    //! pSearchParam und pSearchText nicht vergleichen
}

utl::TextSearch* ScQueryEntry::GetSearchTextPtr( bool bCaseSens ) const
{
    if ( !pSearchParam )
    {
        const rtl::OUString& rStr = maQueryItems[0].maString;
        pSearchParam = new utl::SearchParam(
            rStr, utl::SearchParam::SRCH_REGEXP, bCaseSens, false, false);
        pSearchText = new utl::TextSearch( *pSearchParam, *ScGlobal::pCharClass );
    }
    return pSearchText;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
