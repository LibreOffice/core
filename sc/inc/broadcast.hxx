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

#include <map>
#include <variant>
#include <ostream>

class ScFormulaCell;
class SvtListener;

namespace sc
{
class FormulaGroupAreaListener;

struct BroadcasterState
{
    struct CellListener
    {
        using DataType = std::variant<const ScFormulaCell*, const SvtListener*>;

        DataType pData;

        CellListener(const ScFormulaCell* p);
        CellListener(const SvtListener* p);
    };

    struct AreaListener
    {
        using DataType = std::variant<const ScFormulaCell*, const sc::FormulaGroupAreaListener*,
                                      const SvtListener*>;

        DataType pData;

        AreaListener(const ScFormulaCell* p);
        AreaListener(const sc::FormulaGroupAreaListener* p);
        AreaListener(const SvtListener* p);
    };

    std::map<ScAddress, std::vector<CellListener>> aCellListenerStore;
    std::map<ScRange, std::vector<AreaListener>> aAreaListenerStore;

    /**
     * Check if a formula cell listens on a single cell.
     */
    bool hasFormulaCellListener(const ScAddress& rBroadcasterPos,
                                const ScAddress& rFormulaPos) const;

    /**
     * Check if a formula cell listens on a single range.
     */
    bool hasFormulaCellListener(const ScRange& rBroadcasterRange,
                                const ScAddress& rFormulaPos) const;

    /**
     * Dump all broadcaster state in YAML format.
     */
    void dump(std::ostream& rStrm, const ScDocument* pDoc = nullptr) const;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
