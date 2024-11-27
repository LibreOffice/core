/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "global.hxx"
#include <memory>
#include <span>

namespace com::sun::star::sheet { struct SubTotalColumn; }
namespace com::sun::star::uno { template <class E> class Sequence; }

struct SC_DLLPUBLIC ScSubTotalParam
{
    SCCOL           nCol1 = 0;                  ///< selected area
    SCROW           nRow1 = 0;
    SCCOL           nCol2 = 0;
    SCROW           nRow2 = 0;
    sal_uInt16      nUserIndex = 0;             ///< index into list
    bool            bRemoveOnly:1     = false;
    bool            bReplace:1        = true;   ///< replace existing results
    bool            bPagebreak:1      = false;  ///< page break at change of group
    bool            bCaseSens:1       = false;
    bool            bDoSort:1         = true;   ///< presort
    bool            bSummaryBelow:1   = true;   ///< Summary below or above (default: below)
    bool            bAscending:1      = true;   ///< sort ascending
    bool            bUserDef:1        = false;  ///< sort user defined
    bool            bIncludePattern:1 = false;  ///< sort formats

    struct SubtotalGroup
    {
        bool  bActive    = false; ///< active groups
        SCCOL nField     = 0;     ///< associated field
        SCCOL nSubTotals = 0;     ///< number of SubTotals

        using Pair = std::pair<SCCOL, ScSubTotalFunc>;
        // array of columns to be calculated, and associated functions
        std::unique_ptr<Pair[]> pSubTotals;

        SubtotalGroup() = default;
        SubtotalGroup(const SubtotalGroup& r);

        SubtotalGroup& operator=(const SubtotalGroup& r);
        bool operator==(const SubtotalGroup& r) const;

        void AllocSubTotals(SCCOL n);
        void SetSubtotals(const css::uno::Sequence<css::sheet::SubTotalColumn>& seq);

        std::span<Pair> subtotals() { return std::span(pSubTotals.get(), nSubTotals); }
        std::span<const Pair> subtotals() const { return std::span(pSubTotals.get(), nSubTotals); }
        SCCOL& col(SCCOL n) { return subtotals()[n].first; }
        SCCOL col(SCCOL n) const { return subtotals()[n].first; }
        ScSubTotalFunc func(SCCOL n) const { return subtotals()[n].second; }
    };
    SubtotalGroup aGroups[MAXSUBTOTAL];

    ScSubTotalParam() = default;
    ScSubTotalParam(const ScSubTotalParam&) = default;

    ScSubTotalParam& operator=(const ScSubTotalParam&) = default;
    inline bool operator==(const ScSubTotalParam&) const = default;
    void SetSubTotals( sal_uInt16 nGroup,
                       const SCCOL* ptrSubTotals,
                       const ScSubTotalFunc* ptrFunctions,
                       sal_uInt16 nCount );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
