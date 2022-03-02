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
#include "SparklineGroup.hxx"
#include "rangelst.hxx"
#include <memory>

namespace sc
{
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
    Sparkline(SCCOL nColumn, SCROW nRow, std::shared_ptr<SparklineGroup> const& pSparklineGroup)
        : m_nColumn(nColumn)
        , m_nRow(nRow)
        , m_pSparklineGroup(pSparklineGroup)
    {
    }

    Sparkline(const Sparkline&) = delete;
    Sparkline& operator=(const Sparkline&) = delete;

    void setInputRange(ScRangeList const& rInputRange) { m_aInputRange = rInputRange; }

    ScRangeList const& getInputRange() { return m_aInputRange; }

    std::shared_ptr<SparklineGroup> const& getSparklineGroup() { return m_pSparklineGroup; }

    SCCOL getColumn() { return m_nColumn; }

    SCROW getRow() { return m_nRow; }
};

/** Contains a list of all created sparklines */
class SC_DLLPUBLIC SparklineList
{
private:
    std::vector<std::weak_ptr<Sparkline>> m_pSparklines;

public:
    SparklineList() {}

    void addSparkline(std::shared_ptr<Sparkline> const& pSparkline)
    {
        m_pSparklines.push_back(pSparkline);
    }

    std::vector<std::shared_ptr<Sparkline>> getSparklines()
    {
        std::vector<std::shared_ptr<Sparkline>> toReturn;

        std::vector<std::weak_ptr<Sparkline>>::iterator aIter;
        for (aIter = m_pSparklines.begin(); aIter != m_pSparklines.end();)
        {
            if (auto aSparkline = aIter->lock())
            {
                toReturn.push_back(aSparkline);
                aIter++;
            }
            else
            {
                aIter = m_pSparklines.erase(aIter);
            }
        }

        return toReturn;
    }
};

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
