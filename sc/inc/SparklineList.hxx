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
#include <memory>
#include <map>

#include "Sparkline.hxx"
#include "SparklineGroup.hxx"

namespace sc
{
/** Tracks and gathers all created sparklines and sparkline groups.
 *
 * All the collections of sparkline groups and sparklines don't take
 * the ownership of the pointers.
 */
class SC_DLLPUBLIC SparklineList
{
private:
    std::vector<std::weak_ptr<SparklineGroup>> m_aSparklineGroups;
    std::map<std::weak_ptr<SparklineGroup>, std::vector<std::weak_ptr<Sparkline>>,
             std::owner_less<>>
        m_aSparklineGroupMap;

public:
    SparklineList();

    void addSparkline(std::shared_ptr<Sparkline> const& pSparkline);
    void removeSparkline(std::shared_ptr<Sparkline> const& pSparkline);

    std::vector<std::shared_ptr<SparklineGroup>> getSparklineGroups();

    std::vector<std::shared_ptr<Sparkline>>
    getSparklinesFor(std::shared_ptr<SparklineGroup> const& pSparklineGroup);
};

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
