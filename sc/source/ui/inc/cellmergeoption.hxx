/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       [ Kohei Yoshida <kyoshida@novell.com> ]
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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

    explicit ScCellMergeOption();
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
