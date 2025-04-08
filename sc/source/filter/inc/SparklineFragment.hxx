/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "excelhandlers.hxx"
#include <oox/core/contexthandler.hxx>
#include <SparklineGroup.hxx>

#include <vector>
#include <memory>

namespace oox::xls
{
/** Transitional sparkline data */
class Sparkline
{
public:
    ScRangeList m_aInputRange;
    ScRangeList m_aTargetRange;
    Sparkline() {}
};

/** Transitional sparkline group data */
class SparklineGroup
{
private:
    std::vector<Sparkline> m_aSparklines;

    std::shared_ptr<sc::SparklineGroup> m_pSparklineGroup;

public:
    SparklineGroup()
        : m_pSparklineGroup(new sc::SparklineGroup)
    {
    }

    const std::shared_ptr<sc::SparklineGroup>& getSparklineGroup() { return m_pSparklineGroup; }

    std::vector<Sparkline>& getSparklines() { return m_aSparklines; }
};

/** Handle import of the sparkline, sparkline group and attributes */
class SparklineGroupsContext : public WorksheetContextBase
{
private:
    std::vector<SparklineGroup> m_aSparklineGroups;

public:
    explicit SparklineGroupsContext(WorksheetContextBase& rFragment);

    oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                 const AttributeList& rAttribs) override;
    void onStartElement(const AttributeList& rAttribs) override;
    void onCharacters(const OUString& rCharacters) override;
    void onEndElement() override;

    void insertSparkline(SparklineGroup& rSparklineGroup, Sparkline& rSparkline);
};

} //namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
