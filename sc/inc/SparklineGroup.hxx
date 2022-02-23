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
#include <tools/color.hxx>
#include <optional>

namespace sc
{
/** Common properties for a group of sparklines */
class SC_DLLPUBLIC SparklineGroup
{
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
    OUString m_sMaxAxisType; // individual, group, custom

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

    std::optional<double> m_aManualMax; // if m_sMinAxisType is "custom"
    std::optional<double> m_aManualMin; // if m_sMaxAxisType is "custom"
    OUString m_sUID;

    SparklineGroup() {}

    SparklineGroup(const SparklineGroup&) = delete;
    SparklineGroup& operator=(const SparklineGroup&) = delete;
};

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
