/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SparklineFragment.hxx>
#include <oox/core/contexthandler.hxx>
#include <oox/token/tokens.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/helper/helper.hxx>
#include <oox/helper/attributelist.hxx>
#include <document.hxx>
#include <rangeutl.hxx>

using ::oox::core::ContextHandlerRef;

namespace oox::xls
{
namespace
{
Color getColor(const AttributeList& rAttribs)
{
    if (rAttribs.hasAttribute(XML_rgb))
    {
        return Color(ColorTransparency,
                     rAttribs.getIntegerHex(XML_rgb, sal_Int32(API_RGB_TRANSPARENT)));
    }
    return Color();
}

void addColorsToSparklineGroup(SparklineGroup& rSparklineGroup, sal_Int32 nElement,
                               const AttributeList& rAttribs)
{
    switch (nElement)
    {
        case XLS14_TOKEN(colorSeries):
            rSparklineGroup.m_aColorSeries = getColor(rAttribs);
            break;
        case XLS14_TOKEN(colorNegative):
            rSparklineGroup.m_aColorNegative = getColor(rAttribs);
            break;
        case XLS14_TOKEN(colorAxis):
            rSparklineGroup.m_aColorAxis = getColor(rAttribs);
            break;
        case XLS14_TOKEN(colorMarkers):
            rSparklineGroup.m_aColorMarkers = getColor(rAttribs);
            break;
        case XLS14_TOKEN(colorFirst):
            rSparklineGroup.m_aColorFirst = getColor(rAttribs);
            break;
        case XLS14_TOKEN(colorLast):
            rSparklineGroup.m_aColorLast = getColor(rAttribs);
            break;
        case XLS14_TOKEN(colorHigh):
            rSparklineGroup.m_aColorHigh = getColor(rAttribs);
            break;
        case XLS14_TOKEN(colorLow):
            rSparklineGroup.m_aColorLow = getColor(rAttribs);
            break;
        default:
            break;
    }
}

void addAttributesToSparklineGroup(SparklineGroup& rSparklineGroup, const AttributeList& rAttribs)
{
    auto oManualMax = rAttribs.getDouble(XML_manualMax);
    auto oManualMin = rAttribs.getDouble(XML_manualMin);

    rSparklineGroup.m_fLineWeight = rAttribs.getDouble(XML_lineWeight, 0.75);

    rSparklineGroup.m_sType = rAttribs.getString(XML_type, "line");

    rSparklineGroup.m_bDateAxis = rAttribs.getBool(XML_dateAxis, false);

    rSparklineGroup.m_sDisplayEmptyCellsAs = rAttribs.getString(XML_displayEmptyCellsAs, "zero");

    rSparklineGroup.m_bMarkers = rAttribs.getBool(XML_markers, false);
    rSparklineGroup.m_bHigh = rAttribs.getBool(XML_high, false);
    rSparklineGroup.m_bLow = rAttribs.getBool(XML_low, false);
    rSparklineGroup.m_bFirst = rAttribs.getBool(XML_first, false);
    rSparklineGroup.m_bLast = rAttribs.getBool(XML_last, false);
    rSparklineGroup.m_bNegative = rAttribs.getBool(XML_negative, false);
    rSparklineGroup.m_bDisplayXAxis = rAttribs.getBool(XML_displayXAxis, false);
    rSparklineGroup.m_bDisplayHidden = rAttribs.getBool(XML_displayHidden, false);

    rSparklineGroup.m_sMinAxisType = rAttribs.getString(XML_minAxisType, "individual");
    rSparklineGroup.m_sMaxAxisType = rAttribs.getString(XML_maxAxisType, "individual");

    rSparklineGroup.m_bRightToLeft = rAttribs.getBool(XML_rightToLeft, false);

    rSparklineGroup.m_sUID = rAttribs.getString(XML_uid, OUString());

    if (rSparklineGroup.m_sMaxAxisType == "custom")
        rSparklineGroup.m_aManualMax = oManualMax.get();
    if (rSparklineGroup.m_sMinAxisType == "custom")
        rSparklineGroup.m_aManualMin = oManualMin.get();
}

} // end anonymous namespace

SparklineGroupsContext::SparklineGroupsContext(WorksheetContextBase& rFragment)
    : WorksheetContextBase(rFragment)
{
}

ContextHandlerRef SparklineGroupsContext::onCreateContext(sal_Int32 nElement,
                                                          const AttributeList& rAttribs)
{
    switch (nElement)
    {
        case XLS14_TOKEN(sparklineGroup):
        {
            auto& rLastGroup = m_aSparklineGroups.emplace_back();
            addAttributesToSparklineGroup(rLastGroup, rAttribs);
            return this;
        }
        case XLS14_TOKEN(colorSeries):
        case XLS14_TOKEN(colorNegative):
        case XLS14_TOKEN(colorAxis):
        case XLS14_TOKEN(colorMarkers):
        case XLS14_TOKEN(colorFirst):
        case XLS14_TOKEN(colorLast):
        case XLS14_TOKEN(colorHigh):
        case XLS14_TOKEN(colorLow):
        {
            auto& rLastGroup = m_aSparklineGroups.back();
            addColorsToSparklineGroup(rLastGroup, nElement, rAttribs);
            return this;
        }
        case XLS14_TOKEN(sparklines):
        {
            return this;
        }
        case XLS14_TOKEN(sparkline):
        {
            auto& rLastGroup = m_aSparklineGroups.back();
            rLastGroup.getSparklines().emplace_back();
            return this;
        }
    }
    return this;
}

void SparklineGroupsContext::onStartElement(const AttributeList&) {}

void SparklineGroupsContext::onCharacters(const OUString& rChars)
{
    if (getCurrentElement() == XM_TOKEN(sqref) || getCurrentElement() == XM_TOKEN(f))
    {
        ScDocument& rDocument = getScDocument();
        auto& rLastGroup = m_aSparklineGroups.back();
        auto& rLastSparkline = rLastGroup.getSparklines().back();

        ScRangeList aRange;
        if (ScRangeStringConverter::GetRangeListFromString(aRange, rChars, rDocument,
                                                           formula::FormulaGrammar::CONV_XL_OOX))
        {
            if (!aRange.empty())
            {
                if (getCurrentElement() == XM_TOKEN(sqref))
                {
                    rLastSparkline.m_aTargetRange = aRange;

                    // Need to set the current sheet index to the range as
                    // it is assumed that the address string referes to
                    // the current sheet and is not defined in the string.
                    for (auto& rRange : rLastSparkline.m_aTargetRange)
                    {
                        rRange.aStart.SetTab(getSheetIndex());
                        rRange.aEnd.SetTab(getSheetIndex());
                    }
                }
                else if (getCurrentElement() == XM_TOKEN(f))
                    rLastSparkline.m_aInputRange = aRange;
            }
        }
    }
}

void SparklineGroupsContext::onEndElement() {}

} //namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
