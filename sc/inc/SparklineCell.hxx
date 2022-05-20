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
#include "Sparkline.hxx"
#include <memory>

namespace sc
{
/** Holder of a sparkline, that is connected to a cell specific */
class SC_DLLPUBLIC SparklineCell
{
private:
    std::shared_ptr<Sparkline> m_pSparkline;

public:
    SparklineCell(std::shared_ptr<Sparkline> const& pSparkline)
        : m_pSparkline(pSparkline)
    {
    }

    SparklineCell(const SparklineCell&) = delete;
    SparklineCell& operator=(const SparklineCell&) = delete;

    void setInputRange(ScRangeList const& rInputRange) { m_pSparkline->setInputRange(rInputRange); }

    ScRangeList const& getInputRange() { return m_pSparkline->getInputRange(); }

    std::shared_ptr<SparklineGroup> const& getSparklineGroup() const
    {
        return m_pSparkline->getSparklineGroup();
    }

    std::shared_ptr<Sparkline> const& getSparkline() const { return m_pSparkline; }
};

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
