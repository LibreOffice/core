/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_COLUMNSET_HXX
#define SC_COLUMNSET_HXX

#include "address.hxx"

#include <vector>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

namespace sc {

/**
 * Simple container to keep track of sheet - column pair.
 */
class ColumnSet
{
    typedef boost::unordered_set<SCCOL> ColsType;
    typedef boost::unordered_map<SCTAB, ColsType> TabsType;
    TabsType maTabs;

public:
    void set(SCTAB nTab, SCCOL nCol);
    bool has(SCTAB nTab, SCCOL nCol) const;
    void getColumns(SCTAB nTab, std::vector<SCCOL>& rCols) const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
