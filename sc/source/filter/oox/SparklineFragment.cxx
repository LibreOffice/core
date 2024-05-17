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
#include <Sparkline.hxx>
#include <themebuffer.hxx>
#include <docmodel/color/ComplexColor.hxx>

using ::oox::core::ContextHandlerRef;

namespace oox::xls
{
namespace
{
// TODO: deduplicate with importOOXColor
::Color getColor(const AttributeList& rAttribs, ThemeBuffer const& rThemeBuffer)
{
    if (rAttribs.hasAttribute(XML_rgb))
    {
        return ::Color(ColorAlpha, rAttribs.getIntegerHex(XML_rgb, sal_Int32(API_RGB_TRANSPARENT)));
    }
    else if (rAttribs.hasAttribute(XML_theme))
    {
        sal_uInt32 nThemeIndex = rAttribs.getUnsigned(XML_theme, 0);

        // Excel has a bug in the mapping of index 0, 1, 2 and 3.
        if (nThemeIndex == 0)
            nThemeIndex = 1;
        else if (nThemeIndex == 1)
            nThemeIndex = 0;
        else if (nThemeIndex == 2)
            nThemeIndex = 3;
        else if (nThemeIndex == 3)
            nThemeIndex = 2;

        ::Color aColor = rThemeBuffer.getColorByIndex(nThemeIndex);
        double nTint = rAttribs.getDouble(XML_tint, 0.0);

        if (nTint > 0.0)
            aColor.ApplyTintOrShade(nTint * 10000);
        return aColor;
    }

    return ::Color();
}

model::ComplexColor fillComplexColor(const AttributeList& rAttribs, ThemeBuffer const& rThemeBuffer,
                                     const GraphicHelper& rGraphicHelper)
{
    XlsColor aColor;
    aColor.importColor(rAttribs);
    model::ComplexColor aComplexColor = aColor.createComplexColor(rGraphicHelper, -1);
    ::Color aFinalColor = getColor(rAttribs, rThemeBuffer);
    aComplexColor.setFinalColor(aFinalColor);
    return aComplexColor;
}

void addColorsToSparklineAttributes(sc::SparklineAttributes& rAttributes, sal_Int32 nElement,
                                    const AttributeList& rAttribs, ThemeBuffer& rThemeBuffer,
                                    const GraphicHelper& rHelper)
{
    switch (nElement)
    {
        case XLS14_TOKEN(colorSeries):
            rAttributes.setColorSeries(fillComplexColor(rAttribs, rThemeBuffer, rHelper));
            break;
        case XLS14_TOKEN(colorNegative):
            rAttributes.setColorNegative(fillComplexColor(rAttribs, rThemeBuffer, rHelper));
            break;
        case XLS14_TOKEN(colorAxis):
            rAttributes.setColorAxis(fillComplexColor(rAttribs, rThemeBuffer, rHelper));
            break;
        case XLS14_TOKEN(colorMarkers):
            rAttributes.setColorMarkers(fillComplexColor(rAttribs, rThemeBuffer, rHelper));
            break;
        case XLS14_TOKEN(colorFirst):
            rAttributes.setColorFirst(fillComplexColor(rAttribs, rThemeBuffer, rHelper));
            break;
        case XLS14_TOKEN(colorLast):
            rAttributes.setColorLast(fillComplexColor(rAttribs, rThemeBuffer, rHelper));
            break;
        case XLS14_TOKEN(colorHigh):
            rAttributes.setColorHigh(fillComplexColor(rAttribs, rThemeBuffer, rHelper));
            break;
        case XLS14_TOKEN(colorLow):
            rAttributes.setColorLow(fillComplexColor(rAttribs, rThemeBuffer, rHelper));
            break;
        default:
            break;
    }
}

sc::SparklineType parseSparklineType(std::u16string_view rString)
{
    if (rString == u"column")
        return sc::SparklineType::Column;
    else if (rString == u"stacked")
        return sc::SparklineType::Stacked;
    return sc::SparklineType::Line;
}

sc::DisplayEmptyCellsAs parseDisplayEmptyCellsAs(std::u16string_view rString)
{
    if (rString == u"span")
        return sc::DisplayEmptyCellsAs::Span;
    else if (rString == u"gap")
        return sc::DisplayEmptyCellsAs::Gap;
    return sc::DisplayEmptyCellsAs::Zero;
}

sc::AxisType parseAxisType(std::u16string_view rString)
{
    if (rString == u"group")
        return sc::AxisType::Group;
    else if (rString == u"custom")
        return sc::AxisType::Custom;
    return sc::AxisType::Individual;
}

void addAttributesToSparklineAttributes(sc::SparklineAttributes& rSparklineAttributes,
                                        const AttributeList& rAttribs)
{
    auto oManualMax = rAttribs.getDouble(XML_manualMax);
    auto oManualMin = rAttribs.getDouble(XML_manualMin);

    rSparklineAttributes.setLineWeight(rAttribs.getDouble(XML_lineWeight, 0.75));

    OUString sType = rAttribs.getString(XML_type, u"line"_ustr);
    rSparklineAttributes.setType(parseSparklineType(sType));

    rSparklineAttributes.setDateAxis(rAttribs.getBool(XML_dateAxis, false));

    OUString sDisplayEmptyCellsAs = rAttribs.getString(XML_displayEmptyCellsAs, u"zero"_ustr);
    rSparklineAttributes.setDisplayEmptyCellsAs(parseDisplayEmptyCellsAs(sDisplayEmptyCellsAs));

    rSparklineAttributes.setMarkers(rAttribs.getBool(XML_markers, false));
    rSparklineAttributes.setHigh(rAttribs.getBool(XML_high, false));
    rSparklineAttributes.setLow(rAttribs.getBool(XML_low, false));
    rSparklineAttributes.setFirst(rAttribs.getBool(XML_first, false));
    rSparklineAttributes.setLast(rAttribs.getBool(XML_last, false));
    rSparklineAttributes.setNegative(rAttribs.getBool(XML_negative, false));
    rSparklineAttributes.setDisplayXAxis(rAttribs.getBool(XML_displayXAxis, false));
    rSparklineAttributes.setDisplayHidden(rAttribs.getBool(XML_displayHidden, false));

    OUString sMinAxisType = rAttribs.getString(XML_minAxisType, u"individual"_ustr);
    rSparklineAttributes.setMinAxisType(parseAxisType(sMinAxisType));

    OUString sMaxAxisType = rAttribs.getString(XML_maxAxisType, u"individual"_ustr);
    rSparklineAttributes.setMaxAxisType(parseAxisType(sMaxAxisType));

    rSparklineAttributes.setRightToLeft(rAttribs.getBool(XML_rightToLeft, false));

    if (rSparklineAttributes.getMaxAxisType() == sc::AxisType::Custom)
        rSparklineAttributes.setManualMax(oManualMax.value());
    if (rSparklineAttributes.getMinAxisType() == sc::AxisType::Custom)
        rSparklineAttributes.setManualMin(oManualMin.value());
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
            auto& rSparklineAttributes = rLastGroup.getSparklineGroup()->getAttributes();
            rSparklineAttributes.resetColors();
            addAttributesToSparklineAttributes(rSparklineAttributes, rAttribs);
            OUString sGUID = rAttribs.getString(XR2_TOKEN(uid), OUString());
            tools::Guid aGuid(OUStringToOString(sGUID, RTL_TEXTENCODING_ASCII_US));
            rLastGroup.getSparklineGroup()->setID(aGuid);
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
            auto& rSparklineAttributes = rLastGroup.getSparklineGroup()->getAttributes();
            addColorsToSparklineAttributes(rSparklineAttributes, nElement, rAttribs, getTheme(),
                                           getBaseFilter().getGraphicHelper());
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
                    // it is assumed that the address string refers to
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

void SparklineGroupsContext::onEndElement()
{
    if (getCurrentElement() == XLS14_TOKEN(sparklineGroup))
    {
        auto& rLastGroup = m_aSparklineGroups.back();
        for (Sparkline& rSparkline : rLastGroup.getSparklines())
        {
            insertSparkline(rLastGroup, rSparkline);
        }
    }
}

void SparklineGroupsContext::insertSparkline(SparklineGroup& rSparklineGroup, Sparkline& rSparkline)
{
    auto& rDocument = getScDocument();
    if (rSparkline.m_aTargetRange.size() == 1)
    {
        auto& rRange = rSparkline.m_aTargetRange[0];
        if (rRange.aStart == rRange.aEnd)
        {
            auto pSparklineGroup = rSparklineGroup.getSparklineGroup();
            auto* pCreated = rDocument.CreateSparkline(rRange.aStart, pSparklineGroup);
            pCreated->setInputRange(rSparkline.m_aInputRange);
        }
    }
}

} //namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
