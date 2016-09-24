/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <listenerquery.hxx>
#include <listenerqueryids.hxx>
#include <address.hxx>
#include <rangelst.hxx>

namespace sc {

RefQueryFormulaGroup::RefQueryFormulaGroup() :
    SvtListener::QueryBase(SC_LISTENER_QUERY_FORMULA_GROUP_POS),
    maSkipRange(ScAddress::INITIALIZE_INVALID) {}

RefQueryFormulaGroup::~RefQueryFormulaGroup() {}

void RefQueryFormulaGroup::setSkipRange( const ScRange& rRange )
{
    maSkipRange = rRange;
}

void RefQueryFormulaGroup::add( const ScAddress& rPos )
{
    if (!rPos.IsValid())
        return;

    if (maSkipRange.IsValid() && maSkipRange.In(rPos))
        // This is within the skip range.  Skip it.
        return;

    TabsType::iterator itTab = maTabs.find(rPos.Tab());
    if (itTab == maTabs.end())
    {
        std::pair<TabsType::iterator,bool> r =
            maTabs.insert(TabsType::value_type(rPos.Tab(), ColsType()));
        if (!r.second)
            // Insertion failed.
            return;

        itTab = r.first;
    }

    ColsType& rCols = itTab->second;
    ColsType::iterator itCol = rCols.find(rPos.Col());
    if (itCol == rCols.end())
    {
        std::pair<ColsType::iterator,bool> r =
            rCols.insert(ColsType::value_type(rPos.Col(), ColType()));
        if (!r.second)
            // Insertion failed.
            return;

        itCol = r.first;
    }

    ColType& rCol = itCol->second;
    rCol.push_back(rPos.Row());
}

const RefQueryFormulaGroup::TabsType& RefQueryFormulaGroup::getAllPositions() const
{
    return maTabs;
}

struct QueryRange::Impl
{
    ScRangeList maRanges;
};

QueryRange::QueryRange() :
    SvtListener::QueryBase(SC_LISTENER_QUERY_FORMULA_GROUP_RANGE),
    mpImpl(new Impl) {}

QueryRange::~QueryRange()
{
}

void QueryRange::add( const ScRange& rRange )
{
    mpImpl->maRanges.Join(rRange);
}

void QueryRange::swapRanges( ScRangeList& rRanges )
{
    mpImpl->maRanges.swap(rRanges);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
