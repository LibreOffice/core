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
 * to the SparklineGroup, which inclues common properties of multiple
 * sparklines.
 */
class SC_DLLPUBLIC Sparkline
{
private:
    ScRangeList m_aInputRange;
    std::shared_ptr<SparklineGroup> m_pSparklineGroup;

public:
    Sparkline(std::shared_ptr<SparklineGroup>& pSparklineGroup);

    Sparkline(const Sparkline&) = delete;
    Sparkline& operator=(const Sparkline&) = delete;

    void setInputRange(ScRangeList const& rInputRange) { m_aInputRange = rInputRange; }
    ScRangeList const& getInputRange() { return m_aInputRange; }

    std::shared_ptr<SparklineGroup> const& getSparklineGroup() { return m_pSparklineGroup; }
};

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
