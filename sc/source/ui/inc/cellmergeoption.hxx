/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_CELLMERGEOPTION_HXX
#define SC_CELLMERGEOPTION_HXX

#include "address.hxx"

#include <set>

class ScRange;

struct ScCellMergeOption
{
    ::std::set<SCTAB> maTabs;
    SCCOL mnStartCol;
    SCROW mnStartRow;
    SCCOL mnEndCol;
    SCROW mnEndRow;
    bool mbCenter;

    explicit ScCellMergeOption(const ScRange& rRange);
    explicit ScCellMergeOption(SCCOL nStartCol, SCROW nStartRow,
                               SCCOL nEndCol, SCROW nEndRow,
                               bool bCenter = false);
    explicit ScCellMergeOption(const ScCellMergeOption& r);

    ScRange getSingleRange(SCTAB nTab) const;
    ScRange getFirstSingleRange() const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
