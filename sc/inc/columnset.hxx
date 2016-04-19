/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_COLUMNSET_HXX
#define INCLUDED_SC_INC_COLUMNSET_HXX

#include "address.hxx"

#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace sc {

/**
 * Simple container to keep track of sheet - column pair.
 */
class ColumnSet
{
    typedef std::unordered_set<SCCOL> ColsType;
    typedef std::unordered_map<SCTAB, ColsType> TabsType;
    TabsType maTabs;

public:
    void set(SCTAB nTab, SCCOL nCol);
    void getColumns(SCTAB nTab, std::vector<SCCOL>& rCols) const;
    bool hasTab( SCTAB nTab ) const;
    bool empty() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
