/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "SparklineGroupsExport.hxx"
#include "xmlexprt.hxx"
#include <rangeutl.hxx>
#include <SparklineList.hxx>
#include <document.hxx>

#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
#include <rtl/ustrbuf.hxx>
#include <sax/tools/converter.hxx>
#include <o3tl/unit_conversion.hxx>

using namespace css;
using namespace xmloff::token;

namespace sc
{
SparklineGroupsExport::SparklineGroupsExport(ScXMLExport& rExport, SCTAB nTable)
    : m_rExport(rExport)
    , m_nTable(nTable)
{
}

void SparklineGroupsExport::insertColor(Color aColor, XMLTokenEnum eToken)
{
    OUStringBuffer aStringBuffer;
    if (aColor != COL_TRANSPARENT)
    {
        sax::Converter::convertColor(aStringBuffer, aColor);
        m_rExport.AddAttribute(XML_NAMESPACE_CALC_EXT, eToken, aStringBuffer.makeStringAndClear());
    }
}

void SparklineGroupsExport::insertBool(bool bValue, XMLTokenEnum eToken)
{
    if (bValue)
        m_rExport.AddAttribute(XML_NAMESPACE_CALC_EXT, eToken, "true");
}

void SparklineGroupsExport::addSparklineAttributes(Sparkline const& rSparkline)
{
    auto const* pDocument = m_rExport.GetDocument();

    {
        OUString sAddressString;
        ScAddress aAddress(rSparkline.getColumn(), rSparkline.getRow(), m_nTable);
        ScRangeStringConverter::GetStringFromAddress(sAddressString, aAddress, pDocument,
                                                     formula::FormulaGrammar::CONV_OOO);
        m_rExport.AddAttribute(XML_NAMESPACE_CALC_EXT, XML_CELL_ADDRESS, sAddressString);
    }

    {
        OUString sDataRangeString;
        ScRangeList const& rRangeList = rSparkline.getInputRange();
        ScRangeStringConverter::GetStringFromRangeList(sDataRangeString, &rRangeList, pDocument,
                                                       formula::FormulaGrammar::CONV_OOO);
        m_rExport.AddAttribute(XML_NAMESPACE_CALC_EXT, XML_DATA_RANGE, sDataRangeString);
    }
}

namespace
{
OUString convertSparklineType(sc::SparklineType eType)
{
    switch (eType)
    {
        case sc::SparklineType::Line:
            return u"line";
        case sc::SparklineType::Column:
            return u"column";
        case sc::SparklineType::Stacked:
            return u"stacked";
    }
    return u"";
}

OUString convertDisplayEmptyCellsAs(sc::DisplayEmptyCellsAs eType)
{
    switch (eType)
    {
        case sc::DisplayEmptyCellsAs::Zero:
            return u"zero";
        case sc::DisplayEmptyCellsAs::Gap:
            return u"gap";
        case sc::DisplayEmptyCellsAs::Span:
            return u"span";
    }
    return u"";
}

OUString convertAxisType(sc::AxisType eType)
{
    switch (eType)
    {
        case sc::AxisType::Individual:
            return u"individual";
        case sc::AxisType::Group:
            return u"group";
        case sc::AxisType::Custom:
            return u"custom";
    }
    return u"";
}

} // end anonymous ns

void SparklineGroupsExport::addSparklineGroupAttributes(SparklineAttributes const& rAttributes)
{
    OUString sType = convertSparklineType(rAttributes.getType());
    m_rExport.AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, sType);

    // Line Weight = Line Width in ODF

    m_rExport.AddAttribute(XML_NAMESPACE_CALC_EXT, XML_LINE_WIDTH,
                           OUString::number(rAttributes.getLineWeight()) + "pt");

    insertBool(rAttributes.isDateAxis(), XML_DATE_AXIS);

    OUString sDisplayEmptyCellsAs
        = convertDisplayEmptyCellsAs(rAttributes.getDisplayEmptyCellsAs());
    m_rExport.AddAttribute(XML_NAMESPACE_CALC_EXT, XML_DISPLAY_EMPTY_CELLS_AS,
                           sDisplayEmptyCellsAs);

    insertBool(rAttributes.isMarkers(), XML_MARKERS);
    insertBool(rAttributes.isHigh(), XML_HIGH);
    insertBool(rAttributes.isLow(), XML_LOW);
    insertBool(rAttributes.isFirst(), XML_FIRST);
    insertBool(rAttributes.isLast(), XML_LAST);
    insertBool(rAttributes.isNegative(), XML_NEGATIVE);
    insertBool(rAttributes.shouldDisplayXAxis(), XML_DISPLAY_X_AXIS);
    insertBool(rAttributes.shouldDisplayHidden(), XML_DISPLAY_HIDDEN);

    OUString sMinAxisType = convertAxisType(rAttributes.getMinAxisType());
    m_rExport.AddAttribute(XML_NAMESPACE_CALC_EXT, XML_MIN_AXIS_TYPE, sMinAxisType);

    OUString sMaxAxisType = convertAxisType(rAttributes.getMaxAxisType());
    m_rExport.AddAttribute(XML_NAMESPACE_CALC_EXT, XML_MAX_AXIS_TYPE, sMaxAxisType);

    insertBool(rAttributes.isRightToLeft(), XML_RIGHT_TO_LEFT);

    if (rAttributes.getManualMax() && rAttributes.getMaxAxisType() == sc::AxisType::Custom)
        m_rExport.AddAttribute(XML_NAMESPACE_CALC_EXT, XML_MANUAL_MAX,
                               OUString::number(*rAttributes.getManualMax()));

    if (rAttributes.getManualMin() && rAttributes.getMinAxisType() == sc::AxisType::Custom)
        m_rExport.AddAttribute(XML_NAMESPACE_CALC_EXT, XML_MANUAL_MIN,
                               OUString::number(*rAttributes.getManualMin()));

    insertColor(rAttributes.getColorSeries(), XML_COLOR_SERIES);
    insertColor(rAttributes.getColorNegative(), XML_COLOR_NEGATIVE);
    insertColor(rAttributes.getColorAxis(), XML_COLOR_AXIS);
    insertColor(rAttributes.getColorMarkers(), XML_COLOR_MARKERS);
    insertColor(rAttributes.getColorFirst(), XML_COLOR_FIRST);
    insertColor(rAttributes.getColorLast(), XML_COLOR_LAST);
    insertColor(rAttributes.getColorHigh(), XML_COLOR_HIGH);
    insertColor(rAttributes.getColorLow(), XML_COLOR_LOW);
}

void SparklineGroupsExport::addSparklineGroup(
    std::shared_ptr<SparklineGroup> const& pSparklineGroup,
    std::vector<std::shared_ptr<Sparkline>> const& rSparklines)
{
    auto const& rAttributes = pSparklineGroup->getAttributes();

    OUString sID = pSparklineGroup->getID().getOUString();
    m_rExport.AddAttribute(XML_NAMESPACE_CALC_EXT, XML_ID, sID);

    addSparklineGroupAttributes(rAttributes);

    SvXMLElementExport aElementSparklineGroup(m_rExport, XML_NAMESPACE_CALC_EXT,
                                              XML_SPARKLINE_GROUP, true, true);

    SvXMLElementExport aElementSparklines(m_rExport, XML_NAMESPACE_CALC_EXT, XML_SPARKLINES, true,
                                          true);

    for (auto const& rSparkline : rSparklines)
    {
        addSparklineAttributes(*rSparkline);
        SvXMLElementExport aElementSparkline(m_rExport, XML_NAMESPACE_CALC_EXT, XML_SPARKLINE, true,
                                             true);
    }
}

void SparklineGroupsExport::write()
{
    auto* pDocument = m_rExport.GetDocument();
    if (sc::SparklineList* pSparklineList = pDocument->GetSparklineList(m_nTable))
    {
        auto const& aSparklineGroups = pSparklineList->getSparklineGroups();
        if (!aSparklineGroups.empty())
        {
            SvXMLElementExport aElement(m_rExport, XML_NAMESPACE_CALC_EXT, XML_SPARKLINE_GROUPS,
                                        true, true);

            for (auto const& pSparklineGroup : aSparklineGroups)
            {
                auto const& aSparklines = pSparklineList->getSparklinesFor(pSparklineGroup);
                addSparklineGroup(pSparklineGroup, aSparklines);
            }
        }
    }
}

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
