/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include "scdllapi.h"
#include "rangelst.hxx"
#include <memory>
#include <utility>

namespace sc
{
class SparklineGroup;

/** Sparkline data, used for rendering the content of a cell
 *
 * Contains the input range of the data that is being drawn and a reference
 * to the SparklineGroup, which includes common properties of multiple
 * sparklines.
 */
class SC_DLLPUBLIC Sparkline
{
    SCCOL m_nColumn;
    SCROW m_nRow;

    ScRangeList m_aInputRange;
    std::shared_ptr<SparklineGroup> m_pSparklineGroup;

public:
    Sparkline(SCCOL nColumn, SCROW nRow, std::shared_ptr<SparklineGroup> pSparklineGroup)
        : m_nColumn(nColumn)
        , m_nRow(nRow)
        , m_pSparklineGroup(std::move(pSparklineGroup))
    {
    }

    Sparkline(const Sparkline&) = delete;
    Sparkline& operator=(const Sparkline&) = delete;

    void setInputRange(ScRangeList const& rInputRange) { m_aInputRange = rInputRange; }

    ScRangeList const& getInputRange() const { return m_aInputRange; }

    std::shared_ptr<SparklineGroup> const& getSparklineGroup() const { return m_pSparklineGroup; }

    SCCOL getColumn() const { return m_nColumn; }

    SCROW getRow() const { return m_nRow; }
};

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
