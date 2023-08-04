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
#include <xmloff/XMLComplexColorContext.hxx>

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
sc::SparklineType parseSparklineType(std::string_view aString)
{
    if (aString == "column")
        return sc::SparklineType::Column;
    else if (aString == "stacked")
        return sc::SparklineType::Stacked;
    return sc::SparklineType::Line;
}

sc::DisplayEmptyCellsAs parseDisplayEmptyCellsAs(std::string_view aString)
{
    if (aString == "span")
        return sc::DisplayEmptyCellsAs::Span;
    else if (aString == "gap")
        return sc::DisplayEmptyCellsAs::Gap;
    return sc::DisplayEmptyCellsAs::Zero;
}

sc::AxisType parseAxisType(std::string_view aString)
{
    if (aString == "group")
        return sc::AxisType::Group;
    else if (aString == "custom")
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
                tools::Guid aGuid(rIter.toView());
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
                rAttributes.setType(parseSparklineType(rIter.toView()));
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
                auto eDisplayEmptyCellsAs = parseDisplayEmptyCellsAs(rIter.toView());
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
                rAttributes.setMinAxisType(parseAxisType(rIter.toView()));
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_MAX_AXIS_TYPE):
            {
                rAttributes.setMaxAxisType(parseAxisType(rIter.toView()));
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
                maSeriesColor = COL_TRANSPARENT;
                sax::Converter::convertColor(maSeriesColor, rIter.toView());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_COLOR_NEGATIVE):
            {
                maNegativeColor = COL_TRANSPARENT;
                sax::Converter::convertColor(maNegativeColor, rIter.toView());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_COLOR_AXIS):
            {
                maAxisColor = COL_TRANSPARENT;
                sax::Converter::convertColor(maAxisColor, rIter.toView());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_COLOR_MARKERS):
            {
                maMarkersColor = COL_TRANSPARENT;
                sax::Converter::convertColor(maMarkersColor, rIter.toView());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_COLOR_FIRST):
            {
                maFirstColor = COL_TRANSPARENT;
                sax::Converter::convertColor(maFirstColor, rIter.toView());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_COLOR_LAST):
            {
                maLastColor = COL_TRANSPARENT;
                sax::Converter::convertColor(maLastColor, rIter.toView());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_COLOR_HIGH):
            {
                maHighColor = COL_TRANSPARENT;
                sax::Converter::convertColor(maHighColor, rIter.toView());
                break;
            }
            case XML_ELEMENT(CALC_EXT, XML_COLOR_LOW):
            {
                maLowColor = COL_TRANSPARENT;
                sax::Converter::convertColor(maLowColor, rIter.toView());
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
            maAxisComplexColor = model::ComplexColor();
            maFirstComplexColor = model::ComplexColor();
            maLastComplexColor = model::ComplexColor();
            maHighComplexColor = model::ComplexColor();
            maLowComplexColor = model::ComplexColor();
            maSeriesComplexColor = model::ComplexColor();
            maNegativeComplexColor = model::ComplexColor();
            maMarkersComplexColor = model::ComplexColor();

            fillSparklineGroupID(xAttrList);
            fillSparklineGroupAttributes(xAttrList);
            pContext = this;
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_SPARKLINES):
        {
            pContext = this;
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_SPARKLINE):
        {
            SparklineImportData& rImportData = m_aCurrentSparklineDataList.emplace_back();
            fillSparklineAttributes(rImportData, xAttrList);
            pContext = this;
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_SPARKLINE_AXIS_COMPLEX_COLOR):
        {
            pContext = new XMLComplexColorContext(GetImport(), maAxisComplexColor, xAttrList);
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_SPARKLINE_FIRST_COMPLEX_COLOR):
        {
            pContext = new XMLComplexColorContext(GetImport(), maFirstComplexColor, xAttrList);
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_SPARKLINE_LAST_COMPLEX_COLOR):
        {
            pContext = new XMLComplexColorContext(GetImport(), maLastComplexColor, xAttrList);
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_SPARKLINE_HIGH_COMPLEX_COLOR):
        {
            pContext = new XMLComplexColorContext(GetImport(), maHighComplexColor, xAttrList);
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_SPARKLINE_LOW_COMPLEX_COLOR):
        {
            pContext = new XMLComplexColorContext(GetImport(), maLowComplexColor, xAttrList);
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_SPARKLINE_SERIES_COMPLEX_COLOR):
        {
            pContext = new XMLComplexColorContext(GetImport(), maSeriesComplexColor, xAttrList);
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_SPARKLINE_NEGATIVE_COMPLEX_COLOR):
        {
            pContext = new XMLComplexColorContext(GetImport(), maNegativeComplexColor, xAttrList);
        }
        break;
        case XML_ELEMENT(CALC_EXT, XML_SPARKLINE_MARKERS_COMPLEX_COLOR):
        {
            pContext = new XMLComplexColorContext(GetImport(), maMarkersComplexColor, xAttrList);
        }
        break;
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

namespace
{
model::ComplexColor combineComplexColorAndColor(model::ComplexColor& rComplexColor, Color aColor)
{
    if (rComplexColor.getType() != model::ColorType::Unused)
        rComplexColor.setFinalColor(aColor);
    else if (aColor != COL_TRANSPARENT)
        rComplexColor = model::ComplexColor::setRGB(aColor);
    return rComplexColor;
}
} // end anonymous namespace

void SAL_CALL SparklineGroupsImportContext::endFastElement(sal_Int32 nElement)
{
    switch (nElement)
    {
        case XML_ELEMENT(CALC_EXT, XML_SPARKLINE_GROUP):
        {
            sc::SparklineAttributes& rAttributes = m_pCurrentSparklineGroup->getAttributes();
            {
                rAttributes.setColorAxis(
                    combineComplexColorAndColor(maAxisComplexColor, maAxisColor));
                rAttributes.setColorFirst(
                    combineComplexColorAndColor(maFirstComplexColor, maFirstColor));
                rAttributes.setColorLast(
                    combineComplexColorAndColor(maLastComplexColor, maLastColor));
                rAttributes.setColorHigh(
                    combineComplexColorAndColor(maHighComplexColor, maHighColor));
                rAttributes.setColorLow(combineComplexColorAndColor(maLowComplexColor, maLowColor));
                rAttributes.setColorSeries(
                    combineComplexColorAndColor(maSeriesComplexColor, maSeriesColor));
                rAttributes.setColorNegative(
                    combineComplexColorAndColor(maNegativeComplexColor, maNegativeColor));
                rAttributes.setColorMarkers(
                    combineComplexColorAndColor(maMarkersComplexColor, maMarkersColor));
            }
            insertSparklines();
            m_pCurrentSparklineGroup.reset();
            m_aCurrentSparklineDataList.clear();
            break;
        }
    }
}

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
