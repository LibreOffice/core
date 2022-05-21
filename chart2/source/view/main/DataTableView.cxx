/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DataTableView.hxx>
#include <VSeriesPlotter.hxx>
#include <ShapeFactory.hxx>
#include <ExplicitCategoriesProvider.hxx>
#include <ChartModel.hxx>

#include <svx/svdotable.hxx>

#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/util/XBroadcaster.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <o3tl/unit_conversion.hxx>

using namespace css;

namespace chart
{
DataTableView::DataTableView(rtl::Reference<::chart::ChartModel> const& xChartModel,
                             rtl::Reference<DataTable> const& rDataTableModel)
    : m_xChartModel(xChartModel)
    , m_xDataTableModel(rDataTableModel)
{
    uno::Reference<beans::XPropertySet> xProp(m_xDataTableModel);
    m_aLineProperties.initFromPropertySet(xProp);
}

namespace
{
void setTopCell(uno::Reference<beans::XPropertySet>& xPropertySet)
{
    xPropertySet->setPropertyValue("FillColor", uno::Any(Color(0xFFFFFF)));
    xPropertySet->setPropertyValue("TextVerticalAdjust", uno::Any(drawing::TextVerticalAdjust_TOP));
    xPropertySet->setPropertyValue("ParaAdjust", uno::Any(style::ParagraphAdjust_CENTER));

    table::BorderLine2 aBorderLine;
    aBorderLine.LineWidth = 0;
    aBorderLine.Color = 0x000000;

    xPropertySet->setPropertyValue("TopBorder", uno::Any(aBorderLine));
    xPropertySet->setPropertyValue("LeftBorder", uno::Any(aBorderLine));
}

void copyProperty(uno::Reference<beans::XPropertySet>& xOut,
                  uno::Reference<beans::XPropertySet>& xIn, OUString const& sPropertyName)
{
    xOut->setPropertyValue(sPropertyName, xIn->getPropertyValue(sPropertyName));
}
}

void DataTableView::setCellDefaults(uno::Reference<beans::XPropertySet>& xPropertySet, bool bLeft,
                                    bool bTop, bool bRight, bool bBottom)
{
    uno::Reference<beans::XPropertySet> xDataTableProperties = m_xDataTableModel.get();

    copyProperty(xPropertySet, xDataTableProperties, "CharColor");
    copyProperty(xPropertySet, xDataTableProperties, "CharFontFamily");
    copyProperty(xPropertySet, xDataTableProperties, "CharFontFamilyAsian");
    copyProperty(xPropertySet, xDataTableProperties, "CharFontFamilyComplex");
    copyProperty(xPropertySet, xDataTableProperties, "CharFontCharSet");
    copyProperty(xPropertySet, xDataTableProperties, "CharFontCharSetAsian");
    copyProperty(xPropertySet, xDataTableProperties, "CharFontCharSetComplex");
    copyProperty(xPropertySet, xDataTableProperties, "CharFontName");
    copyProperty(xPropertySet, xDataTableProperties, "CharFontNameAsian");
    copyProperty(xPropertySet, xDataTableProperties, "CharFontNameComplex");
    copyProperty(xPropertySet, xDataTableProperties, "CharFontPitch");
    copyProperty(xPropertySet, xDataTableProperties, "CharFontPitchAsian");
    copyProperty(xPropertySet, xDataTableProperties, "CharFontPitchComplex");
    copyProperty(xPropertySet, xDataTableProperties, "CharFontStyleName");
    copyProperty(xPropertySet, xDataTableProperties, "CharFontStyleNameAsian");
    copyProperty(xPropertySet, xDataTableProperties, "CharFontStyleNameComplex");

    copyProperty(xPropertySet, xDataTableProperties, "CharHeight");
    copyProperty(xPropertySet, xDataTableProperties, "CharHeightAsian");
    copyProperty(xPropertySet, xDataTableProperties, "CharHeightComplex");
    copyProperty(xPropertySet, xDataTableProperties, "CharKerning");
    copyProperty(xPropertySet, xDataTableProperties, "CharLocale");
    copyProperty(xPropertySet, xDataTableProperties, "CharLocaleAsian");
    copyProperty(xPropertySet, xDataTableProperties, "CharLocaleComplex");
    copyProperty(xPropertySet, xDataTableProperties, "CharPosture");
    copyProperty(xPropertySet, xDataTableProperties, "CharPostureAsian");
    copyProperty(xPropertySet, xDataTableProperties, "CharPostureComplex");
    copyProperty(xPropertySet, xDataTableProperties, "CharRelief");
    copyProperty(xPropertySet, xDataTableProperties, "CharShadowed");
    copyProperty(xPropertySet, xDataTableProperties, "CharStrikeout");
    copyProperty(xPropertySet, xDataTableProperties, "CharUnderline");
    copyProperty(xPropertySet, xDataTableProperties, "CharUnderlineColor");
    copyProperty(xPropertySet, xDataTableProperties, "CharUnderlineHasColor");
    copyProperty(xPropertySet, xDataTableProperties, "CharOverline");
    copyProperty(xPropertySet, xDataTableProperties, "CharOverlineColor");
    copyProperty(xPropertySet, xDataTableProperties, "CharOverlineHasColor");
    copyProperty(xPropertySet, xDataTableProperties, "CharWeight");
    copyProperty(xPropertySet, xDataTableProperties, "CharWeightAsian");
    copyProperty(xPropertySet, xDataTableProperties, "CharWeightComplex");
    copyProperty(xPropertySet, xDataTableProperties, "CharWordMode");

    float fFontHeight = 0.0;
    xDataTableProperties->getPropertyValue("CharHeight") >>= fFontHeight;
    fFontHeight = o3tl::convert(fFontHeight, o3tl::Length::pt, o3tl::Length::mm100);
    uno::Any aXDistanceAny(sal_Int32(std::round(fFontHeight * 0.18f)));
    uno::Any aYDistanceAny(sal_Int32(std::round(fFontHeight * 0.30f)));
    xPropertySet->setPropertyValue("TextLeftDistance", aXDistanceAny);
    xPropertySet->setPropertyValue("TextRightDistance", aXDistanceAny);
    xPropertySet->setPropertyValue("TextUpperDistance", aYDistanceAny);
    xPropertySet->setPropertyValue("TextLowerDistance", aYDistanceAny);

    xPropertySet->setPropertyValue("FillColor", uno::Any(Color(0xFFFFFF)));
    xPropertySet->setPropertyValue("TextVerticalAdjust", uno::Any(drawing::TextVerticalAdjust_TOP));
    xPropertySet->setPropertyValue("ParaAdjust", uno::Any(style::ParagraphAdjust_CENTER));

    drawing::LineStyle eStyle = drawing::LineStyle_NONE;
    m_aLineProperties.LineStyle >>= eStyle;

    if (eStyle != drawing::LineStyle_NONE)
    {
        table::BorderLine2 aBorderLine;

        sal_Int32 nWidth = 0;
        m_aLineProperties.Width >>= nWidth;
        aBorderLine.LineWidth = o3tl::convert(nWidth, o3tl::Length::mm100, o3tl::Length::twip);

        sal_Int32 nColor = 0;
        m_aLineProperties.Color >>= nColor;
        aBorderLine.Color = nColor;

        aBorderLine.LineStyle = table::BorderLineStyle::SOLID;

        if (eStyle == drawing::LineStyle_DASH)
        {
            OUString aDashName;
            m_aLineProperties.DashName >>= aDashName;
            if (!aDashName.isEmpty() && m_xChartModel.is())
            {
                uno::Reference<container::XNameContainer> xDashTable(
                    m_xChartModel->createInstance("com.sun.star.drawing.DashTable"),
                    uno::UNO_QUERY);
                if (xDashTable.is() && xDashTable->hasByName(aDashName))
                {
                    drawing::LineDash aLineDash;
                    xDashTable->getByName(aDashName) >>= aLineDash;

                    if (aLineDash.Dots == 0 && aLineDash.Dashes == 0)
                        aBorderLine.LineStyle = table::BorderLineStyle::SOLID;
                    else if (aLineDash.Dots == 1 && aLineDash.Dashes == 0)
                        aBorderLine.LineStyle = table::BorderLineStyle::DOTTED;
                    else if (aLineDash.Dots == 0 && aLineDash.Dashes == 1)
                        aBorderLine.LineStyle = table::BorderLineStyle::DASHED;
                    else if (aLineDash.Dots == 1 && aLineDash.Dashes == 1)
                        aBorderLine.LineStyle = table::BorderLineStyle::DASH_DOT;
                    else if (aLineDash.Dots == 2 && aLineDash.Dashes == 1)
                        aBorderLine.LineStyle = table::BorderLineStyle::DASH_DOT_DOT;
                    else
                        aBorderLine.LineStyle = table::BorderLineStyle::DASHED;
                }
            }
        }

        if (bLeft)
            xPropertySet->setPropertyValue("LeftBorder", uno::Any(aBorderLine));
        if (bTop)
            xPropertySet->setPropertyValue("TopBorder", uno::Any(aBorderLine));
        if (bRight)
            xPropertySet->setPropertyValue("RightBorder", uno::Any(aBorderLine));
        if (bBottom)
            xPropertySet->setPropertyValue("BottomBorder", uno::Any(aBorderLine));
    }
}

void DataTableView::createShapes(basegfx::B2DVector const& rStart, basegfx::B2DVector const& rEnd,
                                 sal_Int32 nColumnWidth)
{
    if (!m_xTarget.is())
        return;

    ShapeFactory::removeSubShapes(m_xTarget);
    m_xTableShape = ShapeFactory::createTable(m_xTarget);

    uno::Reference<table::XTable> xTable;
    try
    {
        auto rDelta = rEnd - rStart;
        m_xTableShape->setSize({ basegfx::fround(rDelta.getX()), 0 });
        m_xTableShape->getPropertyValue("Model") >>= xTable;
    }
    catch (const uno::Exception&)
    {
        return;
    }

    if (!xTable.is())
        return;

    uno::Reference<util::XBroadcaster> xBroadcaster(xTable, uno::UNO_QUERY);

    if (!xBroadcaster.is())
        return;

    xBroadcaster->lockBroadcasts();

    bool bHBorder = false;
    bool bVBorder = false;
    bool bOutline = false;

    m_xDataTableModel->getPropertyValue("HBorder") >>= bHBorder;
    m_xDataTableModel->getPropertyValue("VBorder") >>= bVBorder;
    m_xDataTableModel->getPropertyValue("Outline") >>= bOutline;

    sal_Int32 nColumnCount = m_aXValues.size();
    uno::Reference<table::XTableColumns> xTableColumns = xTable->getColumns();
    xTableColumns->insertByIndex(0, nColumnCount);

    sal_Int32 nRowCount = m_aDataSeriesNames.size();
    uno::Reference<table::XTableRows> xTableRows = xTable->getRows();
    xTableRows->insertByIndex(0, nRowCount);

    {
        uno::Reference<table::XCell> xCell = xTable->getCellByPosition(0, 0);
        uno::Reference<beans::XPropertySet> xPropertySet(xCell, uno::UNO_QUERY);
        if (xPropertySet.is())
        {
            setTopCell(xPropertySet);
        }
    }

    sal_Int32 nColumn;
    sal_Int32 nRow;

    nColumn = 1;
    for (auto const& rString : m_aXValues)
    {
        uno::Reference<table::XCell> xCell = xTable->getCellByPosition(nColumn, 0);
        uno::Reference<beans::XPropertySet> xPropertySet(xCell, uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCellTextRange(xCell, uno::UNO_QUERY);
        if (xCellTextRange.is())
        {
            xCellTextRange->setString(rString);
            bool bLeft = bOutline || (bVBorder && nColumn > 1);
            setCellDefaults(xPropertySet, bLeft, bOutline, bOutline, bOutline);
        }
        nColumn++;
    }

    nRow = 1;
    for (auto const& rSeriesName : m_aDataSeriesNames)
    {
        uno::Reference<table::XCell> xCell = xTable->getCellByPosition(0, nRow);
        uno::Reference<beans::XPropertySet> xPropertySet(xCell, uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCellTextRange(xCell, uno::UNO_QUERY);
        if (xCellTextRange.is())
        {
            bool bTop = bOutline || (bHBorder && nRow > 1);
            xCellTextRange->setString(rSeriesName);
            setCellDefaults(xPropertySet, bOutline, bTop, bOutline, bOutline);
        }
        nRow++;
    }

    nRow = 1;
    for (auto const& rSeries : m_pDataSeriesValues)
    {
        nColumn = 1;
        for (auto const& rValue : rSeries)
        {
            uno::Reference<table::XCell> xCell = xTable->getCellByPosition(nColumn, nRow);
            uno::Reference<beans::XPropertySet> xPropertySet(xCell, uno::UNO_QUERY);
            uno::Reference<text::XTextRange> xCellTextRange(xCell, uno::UNO_QUERY);
            if (xCellTextRange.is())
            {
                xCellTextRange->setString(rValue);

                bool bLeft = false;
                bool bTop = false;
                bool bRight = false;
                bool bBottom = false;

                if (nColumn > 1 && bVBorder)
                    bLeft = true;

                if (nRow > 1 && bHBorder)
                    bTop = true;

                if (nRow == nRowCount && bOutline)
                    bBottom = true;

                if (nColumn == nColumnCount && bOutline)
                    bRight = true;

                setCellDefaults(xPropertySet, bLeft, bTop, bRight, bBottom);
            }
            nColumn++;
        }
        nRow++;
    }

    xBroadcaster->unlockBroadcasts();

    auto* pTableObject = static_cast<sdr::table::SdrTableObj*>(m_xTableShape->GetSdrObject());
    pTableObject->DistributeColumns(0, nColumnCount - 1, true, true);

    uno::Reference<beans::XPropertySet> xPropertySet(xTableColumns->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nWidth = 0;
    xPropertySet->getPropertyValue("Width") >>= nWidth;

    m_xTableShape->setPosition(
        { basegfx::fround(rStart.getX() - nWidth), basegfx::fround(rStart.getY()) });

    for (sal_Int32 i = 1; i < xTableColumns->getCount(); ++i)
    {
        xPropertySet.set(xTableColumns->getByIndex(i), uno::UNO_QUERY);
        xPropertySet->setPropertyValue("Width", uno::Any(nColumnWidth));
    }
}

void DataTableView::initializeShapes(const rtl::Reference<SvxShapeGroupAnyD>& xTarget)
{
    m_xTarget = xTarget;
}

void DataTableView::initializeValues(
    std::vector<std::unique_ptr<VSeriesPlotter>>& rSeriesPlotterList)
{
    for (auto& rSeriesPlotter : rSeriesPlotterList)
    {
        for (auto const& rCategory :
             rSeriesPlotter->getExplicitCategoriesProvider()->getSimpleCategories())
        {
            m_aXValues.push_back(rCategory);
        }

        for (auto const& rString : rSeriesPlotter->getAllSeriesNames())
        {
            m_aDataSeriesNames.push_back(rString);
        }

        for (VDataSeries* pSeries : rSeriesPlotter->getAllSeries())
        {
            auto& rValues = m_pDataSeriesValues.emplace_back();
            for (int i = 0; i < pSeries->getTotalPointCount(); i++)
            {
                double nValue = pSeries->getYValue(i);
                rValues.push_back(rSeriesPlotter->getLabelTextForValue(*pSeries, i, nValue, false));
            }
        }
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
