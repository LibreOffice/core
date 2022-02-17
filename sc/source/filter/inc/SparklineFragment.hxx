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

#include <vector>
#include <memory>
#include <optional>

namespace oox
{
class AttributeList;
}

namespace oox::xls
{
class Sparkline
{
public:
    ScRangeList m_aInputRange;
    ScRangeList m_aTargetRange;
    Sparkline() {}
};

class SparklineGroup
{
private:
    std::vector<Sparkline> m_aSparklines;

public:
    Color m_aColorSeries;
    Color m_aColorNegative;
    Color m_aColorAxis;
    Color m_aColorMarkers;
    Color m_aColorFirst;
    Color m_aColorLast;
    Color m_aColorHigh;
    Color m_aColorLow;

    OUString m_sMinAxisType; // individual, group, custom
    OUString m_sMaxAxisType;

    double m_fLineWeight; // In pt

    OUString m_sType; // line, column, stacked

    bool m_bDateAxis;

    OUString m_sDisplayEmptyCellsAs; // span, gap, zero

    bool m_bMarkers;
    bool m_bHigh;
    bool m_bLow;
    bool m_bFirst;
    bool m_bLast;
    bool m_bNegative;
    bool m_bDisplayXAxis;
    bool m_bDisplayHidden;
    bool m_bRightToLeft;

    std::optional<double> m_aManualMax;
    std::optional<double> m_aManualMin;
    OUString m_sUID;

    std::vector<Sparkline>& getSparklines() { return m_aSparklines; }
};

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
};

} //namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
