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

#include "queryentry.hxx"

#include <unotools/textsearch.hxx>

/*
 * dialog returns the special field values "empty"/"not empty"
 * as constants SC_EMPTYFIELDS and SC_NONEMPTYFIELDS respectively in nVal in
 * conjunctions with the flag bQueryByString = FALSE.
 */

#define SC_EMPTYFIELDS      ((double)0x0042)
#define SC_NONEMPTYFIELDS   ((double)0x0043)

bool ScQueryEntry::Item::operator== (const Item& r) const
{
    return meType == r.meType && mfVal == r.mfVal && maString == r.maString && mbMatchEmpty == r.mbMatchEmpty;
}

ScQueryEntry::ScQueryEntry() :
    bDoQuery(false),
    nField(0),
    eOp(SC_EQUAL),
    eConnect(SC_AND),
    pSearchParam(nullptr),
    pSearchText(nullptr),
    maQueryItems(1)
{
}

ScQueryEntry::ScQueryEntry(const ScQueryEntry& r) :
    bDoQuery(r.bDoQuery),
    nField(r.nField),
    eOp(r.eOp),
    eConnect(r.eConnect),
    pSearchParam(nullptr),
    pSearchText(nullptr),
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
    pSearchParam    = nullptr;
    pSearchText     = nullptr;

    return *this;
}

void ScQueryEntry::SetQueryByEmpty()
{
    eOp = SC_EQUAL;
    maQueryItems.resize(1);
    Item& rItem = maQueryItems[0];
    rItem.meType = ByEmpty;
    rItem.maString = svl::SharedString();
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
    rItem.maString = svl::SharedString();
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
    pSearchParam    = nullptr;
    pSearchText     = nullptr;
}

bool ScQueryEntry::operator==( const ScQueryEntry& r ) const
{
    return bDoQuery         == r.bDoQuery
        && eOp              == r.eOp
        && eConnect         == r.eConnect
        && nField           == r.nField
        && maQueryItems   == r.maQueryItems;
    // do not compare pSearchParam and pSearchText!
}

utl::TextSearch* ScQueryEntry::GetSearchTextPtr( bool bCaseSens ) const
{
    if ( !pSearchParam )
    {
        OUString aStr = maQueryItems[0].maString.getString();
        pSearchParam = new utl::SearchParam(
            aStr, utl::SearchParam::SRCH_REGEXP, bCaseSens, false, false);
        pSearchText = new utl::TextSearch( *pSearchParam, *ScGlobal::pCharClass );
    }
    return pSearchText;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
