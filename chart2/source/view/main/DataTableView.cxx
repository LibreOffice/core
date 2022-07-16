/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <DataTableView.hxx>
#include <VSeriesPlotter.hxx>
#include <ShapeFactory.hxx>
#include <ExplicitCategoriesProvider.hxx>

#include <svx/svdotable.hxx>

#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/util/XBroadcaster.hpp>

#include <o3tl/unit_conversion.hxx>

using namespace css;

namespace chart
{
DataTableView::DataTableView() = default;

namespace
{
void setCellDefaults(uno::Reference<beans::XPropertySet>& xPropertySet)
{
    xPropertySet->setPropertyValue("FillColor", uno::Any(Color(0xFFFFFF)));
    xPropertySet->setPropertyValue("TextVerticalAdjust", uno::Any(drawing::TextVerticalAdjust_TOP));
    xPropertySet->setPropertyValue("ParaAdjust", uno::Any(style::ParagraphAdjust_CENTER));

    table::BorderLine2 aBorderLine;
    aBorderLine.LineWidth = o3tl::convert(0.5, o3tl::Length::pt, o3tl::Length::mm100);
    aBorderLine.Color = 0x000000;

    xPropertySet->setPropertyValue("TopBorder", uno::Any(aBorderLine));
    xPropertySet->setPropertyValue("BottomBorder", uno::Any(aBorderLine));
    xPropertySet->setPropertyValue("LeftBorder", uno::Any(aBorderLine));
    xPropertySet->setPropertyValue("RightBorder", uno::Any(aBorderLine));
}

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
}
void DataTableView::createShapes(basegfx::B2DVector const& rStart, basegfx::B2DVector const& rEnd,
                                 sal_Int32 nColumnSize)
{
    if (!m_xTarget.is())
        return;

    ShapeFactory::removeSubShapes(m_xTarget);
    m_xTableShape = ShapeFactory::createTable(m_xTarget);

    uno::Reference<table::XTable> xTable;
    uno::Reference<util::XBroadcaster> xBroadcaster;
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

    if (xTable.is())
        xBroadcaster.set(xTable, uno::UNO_QUERY);

    if (!xBroadcaster.is())
        return;

    xBroadcaster->lockBroadcasts();
    uno::Reference<table::XTableColumns> xTableColumns = xTable->getColumns();
    xTableColumns->insertByIndex(0, m_aXValues.size());

    uno::Reference<table::XTableRows> xTableRows = xTable->getRows();
    xTableRows->insertByIndex(0, m_aDataSeriesNames.size());

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
            setCellDefaults(xPropertySet);
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
            xCellTextRange->setString(rSeriesName);
            setCellDefaults(xPropertySet);
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
                setCellDefaults(xPropertySet);
            }
            nColumn++;
        }
        nRow++;
    }

    xBroadcaster->unlockBroadcasts();

    auto* pTableObject = static_cast<sdr::table::SdrTableObj*>(m_xTableShape->GetSdrObject());
    pTableObject->DistributeColumns(0, pTableObject->getColumnCount() - 1, true, true);

    uno::Reference<beans::XPropertySet> xPropertySet(xTableColumns->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nWidth = 0;
    xPropertySet->getPropertyValue("Width") >>= nWidth;

    m_xTableShape->setPosition(
        { basegfx::fround(rStart.getX() - nWidth), basegfx::fround(rStart.getY()) });

    for (sal_Int32 i = 1; i < xTableColumns->getCount(); ++i)
    {
        xPropertySet.set(xTableColumns->getByIndex(i), uno::UNO_QUERY);
        xPropertySet->setPropertyValue("Width", uno::Any(nColumnSize));
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
