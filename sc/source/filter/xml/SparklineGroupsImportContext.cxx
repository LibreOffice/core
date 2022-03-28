/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SparklineGroupsImportContext.hxx"

#include <sax/tools/converter.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>

#include <document.hxx>
#include <rangeutl.hxx>
#include <Sparkline.hxx>
#include <SparklineGroup.hxx>
#include <SparklineAttributes.hxx>

using namespace xmloff::token;
using namespace css;

namespace sc
{
SparklineGroupsImportContext::SparklineGroupsImportContext(ScXMLImport& rImport)
    : ScXMLImportContext(rImport)
{
}

namespace
{
sc::SparklineType parseSparklineType(OUString const& rString)
{
    if (rString == "column")
        return sc::SparklineType::Column;
    else if (rString == "stacked")
        return sc::SparklineType::Stacked;
    return sc::SparklineType::Line;
}

sc::DisplayEmptyCellsAs parseDisplayEmptyCellsAs(OUString const& rString)
{
    if (rString == "span")
        return sc::DisplayEmptyCellsAs::Span;
    else if (rString == "gap")
        return sc::DisplayEmptyCellsAs::Gap;
    return sc::DisplayEmptyCellsAs::Zero;
}

sc::AxisType parseAxisType(OUString const& rString)
{
    if (rString == "group")
        return sc::AxisType::Group;
    else if (rString == "custom")
        return sc::AxisType::Custom;
    return sc::AxisType::Individual;
}

} // end anonymous namespace

void SparklineGroupsImportContext::fillSparklineGroupID(
    uno::Reference<xml::sax::XFastAttributeList> const& xAttrList)
{
    for (auto& rIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (rIter.getToken())
        {
            case XML_ELEMENT(CALC_EXT, XML_ID):
            {
                OString aString = OUStringToOString(rIter.toString(), RTL_TEXTENCODING_ASCII_US);
                tools::Guid aGuid(aString);
                m_pCurrentSparklineGroup->setID(aGuid);
                break;
            }
        }
    }
}

void SparklineGroupsImportContext::fillSparklineGroupAttributes(
    uno::Reference<xml::sax::XFastAttributeList> const& xAttrList)
{
    sc::SparklineAttributes& rAttributes = m_pCurrentSparklineGroup->getAttributes();

    for (auto& rIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (rIter.getToken())
        {
            case XML_ELEMENT(CALC_EXT, XML_TYPE):
            {
                rAttributes.setType(parseSparklineType(rIter.toString()));
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_LINE_WIDTH):
            {
                OUString sLineWidth = rIter.toString();
                double fLineWidth;
                sal_Int16 const eSrcUnit
                    = ::sax::Converter::GetUnitFromString(sLineWidth, util::MeasureUnit::POINT);
                ::sax::Converter::convertDouble(fLineWidth, sLineWidth, eSrcUnit,
                                                util::MeasureUnit::POINT);
                rAttributes.setLineWeight(fLineWidth);
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_DATE_AXIS):
            {
                rAttributes.setDateAxis(rIter.toBoolean());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_DISPLAY_EMPTY_CELLS_AS):
            {
                auto eDisplayEmptyCellsAs = parseDisplayEmptyCellsAs(rIter.toString());
                rAttributes.setDisplayEmptyCellsAs(eDisplayEmptyCellsAs);
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_MARKERS):
            {
                rAttributes.setMarkers(rIter.toBoolean());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_HIGH):
            {
                rAttributes.setHigh(rIter.toBoolean());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_LOW):
            {
                rAttributes.setLow(rIter.toBoolean());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_FIRST):
            {
                rAttributes.setFirst(rIter.toBoolean());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_LAST):
            {
                rAttributes.setLast(rIter.toBoolean());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_NEGATIVE):
            {
                rAttributes.setNegative(rIter.toBoolean());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_DISPLAY_X_AXIS):
            {
                rAttributes.setDisplayXAxis(rIter.toBoolean());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_DISPLAY_HIDDEN):
            {
                rAttributes.setDisplayHidden(rIter.toBoolean());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_MIN_AXIS_TYPE):
            {
                rAttributes.setMinAxisType(parseAxisType(rIter.toString()));
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_MAX_AXIS_TYPE):
            {
                rAttributes.setMaxAxisType(parseAxisType(rIter.toString()));
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_RIGHT_TO_LEFT):
            {
                rAttributes.setRightToLeft(rIter.toBoolean());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_MANUAL_MAX):
            {
                rAttributes.setManualMax(rIter.toDouble());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_MANUAL_MIN):
            {
                rAttributes.setManualMin(rIter.toDouble());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_COLOR_SERIES):
            {
                Color aColor;
                sax::Converter::convertColor(aColor, rIter.toString());
                rAttributes.setColorSeries(aColor);
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_COLOR_NEGATIVE):
            {
                Color aColor;
                sax::Converter::convertColor(aColor, rIter.toString());
                rAttributes.setColorNegative(aColor);
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_COLOR_AXIS):
            {
                Color aColor;
                sax::Converter::convertColor(aColor, rIter.toString());
                rAttributes.setColorAxis(aColor);
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_COLOR_MARKERS):
            {
                Color aColor;
                sax::Converter::convertColor(aColor, rIter.toString());
                rAttributes.setColorMarkers(aColor);
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_COLOR_FIRST):
            {
                Color aColor;
                sax::Converter::convertColor(aColor, rIter.toString());
                rAttributes.setColorFirst(aColor);
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_COLOR_LAST):
            {
                Color aColor;
                sax::Converter::convertColor(aColor, rIter.toString());
                rAttributes.setColorLast(aColor);
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_COLOR_HIGH):
            {
                Color aColor;
                sax::Converter::convertColor(aColor, rIter.toString());
                rAttributes.setColorHigh(aColor);
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_COLOR_LOW):
            {
                Color aColor;
                sax::Converter::convertColor(aColor, rIter.toString());
                rAttributes.setColorLow(aColor);
                break;
            }
            default:
                break;
        }
    }
}

void SparklineGroupsImportContext::fillSparklineAttributes(
    SparklineImportData& rImportData, uno::Reference<xml::sax::XFastAttributeList> const& xAttrList)
{
    ScDocument* pDocument = GetScImport().GetDocument();

    for (auto& rIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (rIter.getToken())
        {
            case XML_ELEMENT(CALC_EXT, XML_CELL_ADDRESS):
            {
                sal_Int32 nOffset = 0;
                ScRangeStringConverter::GetAddressFromString(
                    rImportData.m_aAddress, rIter.toString(), *pDocument,
                    formula::FormulaGrammar::CONV_OOO, nOffset);
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_DATA_RANGE):
            {
                ScRangeStringConverter::GetRangeListFromString(rImportData.m_aDataRangeList,
                                                               rIter.toString(), *pDocument,
                                                               formula::FormulaGrammar::CONV_OOO);
                break;
            }
            default:
                break;
        }
    }
}

uno::Reference<xml::sax::XFastContextHandler>
    SAL_CALL SparklineGroupsImportContext::createFastChildContext(
        sal_Int32 nElement, uno::Reference<xml::sax::XFastAttributeList> const& xAttrList)
{
    SvXMLImportContext* pContext = nullptr;
    switch (nElement)
    {
        case XML_ELEMENT(CALC_EXT, XML_SPARKLINE_GROUP):
        {
            m_pCurrentSparklineGroup = std::make_shared<sc::SparklineGroup>();
            fillSparklineGroupID(xAttrList);
            fillSparklineGroupAttributes(xAttrList);
            pContext = this;
            break;
        }
        case XML_ELEMENT(CALC_EXT, XML_SPARKLINES):
        {
            pContext = this;
            break;
        }
        case XML_ELEMENT(CALC_EXT, XML_SPARKLINE):
        {
            SparklineImportData& rImportData = m_aCurrentSparklineDataList.emplace_back();
            fillSparklineAttributes(rImportData, xAttrList);
            pContext = this;
            break;
        }
    }

    return pContext;
}

void SparklineGroupsImportContext::insertSparklines()
{
    ScDocument* pDocument = GetScImport().GetDocument();
    for (auto const& rSparklineImportData : m_aCurrentSparklineDataList)
    {
        auto* pSparkline
            = pDocument->CreateSparkline(rSparklineImportData.m_aAddress, m_pCurrentSparklineGroup);
        pSparkline->setInputRange(rSparklineImportData.m_aDataRangeList);
    }
}

void SAL_CALL SparklineGroupsImportContext::endFastElement(sal_Int32 nElement)
{
    switch (nElement)
    {
        case XML_ELEMENT(CALC_EXT, XML_SPARKLINE_GROUP):
        {
            insertSparklines();
            m_pCurrentSparklineGroup.reset();
            m_aCurrentSparklineDataList.clear();
            break;
        }
    }
}

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
