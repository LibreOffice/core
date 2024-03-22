/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "address.hxx"
#include "rangelst.hxx"
#include <svl/listener.hxx>

#include <unordered_map>
#include <vector>

namespace sc
{
/**
 * Used to collect positions of formula cells that belong to a formula
 * group.
 */
class RefQueryFormulaGroup final : public SvtListener::QueryBase
{
public:
    typedef std::vector<SCROW> ColType;
    typedef std::unordered_map<SCCOL, ColType> ColsType;
    typedef std::unordered_map<SCTAB, ColsType> TabsType;

    RefQueryFormulaGroup();
    virtual ~RefQueryFormulaGroup() override;

    void setSkipRange(const ScRange& rRange);
    void add(const ScAddress& rPos);

    /**
     * Row positions in each column may contain duplicates.  Caller must
     * remove duplicates if necessary.
     */
    const TabsType& getAllPositions() const;

private:
    ScRange maSkipRange;
    TabsType maTabs;
};

class QueryRange final : public SvtListener::QueryBase
{
    ScRangeList maRanges;

    QueryRange(const QueryRange&) = delete;
    QueryRange& operator=(const QueryRange&) = delete;

public:
    QueryRange();
    virtual ~QueryRange() override;

    void add(const ScRange& rRange);

    void swapRanges(ScRangeList& rRanges);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
