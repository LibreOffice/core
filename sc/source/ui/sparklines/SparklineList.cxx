/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <SparklineList.hxx>

namespace sc
{
SparklineList::SparklineList() = default;

void SparklineList::addSparkline(std::shared_ptr<Sparkline> const& pSparkline)
{
    auto pWeakGroup = std::weak_ptr<SparklineGroup>(pSparkline->getSparklineGroup());

    auto[iterator, bInserted]
        = m_aSparklineGroupMap.try_emplace(pWeakGroup, std::vector<std::weak_ptr<Sparkline>>());
    iterator->second.push_back(std::weak_ptr<Sparkline>(pSparkline));
    if (bInserted)
        m_aSparklineGroups.push_back(pWeakGroup);
}

void SparklineList::removeSparkline(std::shared_ptr<Sparkline> const& pSparkline)
{
    auto pWeakGroup = std::weak_ptr<SparklineGroup>(pSparkline->getSparklineGroup());
    auto iteratorGroup = m_aSparklineGroupMap.find(pWeakGroup);
    if (iteratorGroup != m_aSparklineGroupMap.end())
    {
        auto& rWeakSparklines = iteratorGroup->second;

        for (auto iterator = rWeakSparklines.begin(); iterator != rWeakSparklines.end();)
        {
            auto pCurrentSparkline = iterator->lock();

            if (pCurrentSparkline && pCurrentSparkline != pSparkline)
            {
                iterator++;
            }
            else
            {
                iterator = rWeakSparklines.erase(iterator);
            }
        }
    }
}

std::vector<std::shared_ptr<SparklineGroup>> SparklineList::getSparklineGroups()
{
    std::vector<std::shared_ptr<SparklineGroup>> toReturn;

    for (auto iterator = m_aSparklineGroups.begin(); iterator != m_aSparklineGroups.end();)
    {
        auto pWeakGroup = *iterator;
        if (auto pSparklineGroup = pWeakGroup.lock())
        {
            toReturn.push_back(std::move(pSparklineGroup));
            iterator++;
        }
        else
        {
            iterator = m_aSparklineGroups.erase(iterator);
        }
    }
    return toReturn;
}

std::vector<std::shared_ptr<Sparkline>>
SparklineList::getSparklinesFor(std::shared_ptr<SparklineGroup> const& pSparklineGroup)
{
    std::vector<std::shared_ptr<Sparkline>> toReturn;

    std::weak_ptr<SparklineGroup> pWeakGroup(pSparklineGroup);
    auto iteratorGroup = m_aSparklineGroupMap.find(pWeakGroup);

    if (iteratorGroup == m_aSparklineGroupMap.end())
        return toReturn;

    auto& rWeakSparklines = iteratorGroup->second;

    for (auto iterator = rWeakSparklines.begin(); iterator != rWeakSparklines.end();)
    {
        if (auto aSparkline = iterator->lock())
        {
            toReturn.push_back(aSparkline);
            iterator++;
        }
        else
        {
            iterator = rWeakSparklines.erase(iterator);
        }
    }

    return toReturn;
}

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
