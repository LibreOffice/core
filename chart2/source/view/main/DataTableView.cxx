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
#include <ObjectIdentifier.hxx>

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
#include <com/sun/star/uno/XComponentContext.hpp>

#include <o3tl/unit_conversion.hxx>

using namespace css;

namespace chart
{
namespace
{
void setTopCell(uno::Reference<beans::XPropertySet>& xPropertySet)
{
    xPropertySet->setPropertyValue("FillColor", uno::makeAny(Color(0xFFFFFF)));
    xPropertySet->setPropertyValue("TextVerticalAdjust",
                                   uno::makeAny(drawing::TextVerticalAdjust_TOP));
    xPropertySet->setPropertyValue("ParaAdjust", uno::makeAny(style::ParagraphAdjust_CENTER));

    table::BorderLine2 aBorderLine;
    aBorderLine.LineWidth = 0;
    aBorderLine.Color = 0x000000;

    xPropertySet->setPropertyValue("TopBorder", uno::makeAny(aBorderLine));
    xPropertySet->setPropertyValue("LeftBorder", uno::makeAny(aBorderLine));
}

void copyProperty(uno::Reference<beans::XPropertySet>& xOut,
                  uno::Reference<beans::XPropertySet>& xIn, OUString const& sPropertyName)
{
    xOut->setPropertyValue(sPropertyName, xIn->getPropertyValue(sPropertyName));
}
} // end anonymous namespace

DataTableView::DataTableView(uno::Reference<chart2::XChartDocument> const& xChartDoc,
                             rtl::Reference<DataTable> const& rDataTableModel,
                             uno::Reference<uno::XComponentContext> const& rComponentContext)
    : m_xChartModel(xChartDoc)
    , m_xDataTableModel(rDataTableModel)
    , m_xComponentContext(rComponentContext)
{
    uno::Reference<beans::XPropertySet> xProp(m_xDataTableModel);
    m_aLineProperties.initFromPropertySet(xProp);
}

void DataTableView::setCellCharAndParagraphProperties(
    uno::Reference<beans::XPropertySet>& xPropertySet)
{
    uno::Reference<beans::XPropertySet> xDataTableProperties(m_xDataTableModel);

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

    xPropertySet->setPropertyValue("ParaAdjust", uno::makeAny(style::ParagraphAdjust_CENTER));
}

void DataTableView::setCellProperties(css::uno::Reference<beans::XPropertySet>& xPropertySet,
                                      bool bLeft, bool bTop, bool bRight, bool bBottom)
{
    xPropertySet->setPropertyValue("FillColor", uno::makeAny(Color(0xFFFFFF)));

    uno::Reference<beans::XPropertySet> xDataTableProperties(m_xDataTableModel);
    float fFontHeight = 0.0;
    xDataTableProperties->getPropertyValue("CharHeight") >>= fFontHeight;
    fFontHeight = o3tl::convert(fFontHeight, o3tl::Length::pt, o3tl::Length::mm100);
    sal_Int32 nXDistance = std::round(fFontHeight * 0.18f);
    sal_Int32 nYDistance = std::round(fFontHeight * 0.30f);

    xPropertySet->setPropertyValue("TextLeftDistance", uno::makeAny(nXDistance));
    xPropertySet->setPropertyValue("TextRightDistance", uno::makeAny(nXDistance));
    xPropertySet->setPropertyValue("TextUpperDistance", uno::makeAny(nYDistance));
    xPropertySet->setPropertyValue("TextLowerDistance", uno::makeAny(nYDistance));

    xPropertySet->setPropertyValue("TextVerticalAdjust",
                                   uno::makeAny(drawing::TextVerticalAdjust_TOP));

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
            uno::Reference<lang::XMultiServiceFactory> xFactory(m_xChartModel, uno::UNO_QUERY);
            if (!aDashName.isEmpty() && xFactory.is())
            {
                uno::Reference<container::XNameContainer> xDashTable(
                    xFactory->createInstance("com.sun.star.drawing.DashTable"), uno::UNO_QUERY);
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
            xPropertySet->setPropertyValue("LeftBorder", uno::makeAny(aBorderLine));
        if (bTop)
            xPropertySet->setPropertyValue("TopBorder", uno::makeAny(aBorderLine));
        if (bRight)
            xPropertySet->setPropertyValue("RightBorder", uno::makeAny(aBorderLine));
        if (bBottom)
            xPropertySet->setPropertyValue("BottomBorder", uno::makeAny(aBorderLine));
    }
}

void DataTableView::createShapes(basegfx::B2DVector const& rStart, basegfx::B2DVector const& rEnd,
                                 sal_Int32 nColumnWidth)
{
    if (!m_xTarget.is())
        return;

    ShapeFactory::removeSubShapes(m_xTarget);
    auto sParticle = ObjectIdentifier::createParticleForDataTable(m_xChartModel);
    auto sCID = ObjectIdentifier::createClassifiedIdentifierForParticle(sParticle);
    m_xTableShape = ShapeFactory::createTable(m_xTarget, sCID);

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

    auto* pTableObject = static_cast<sdr::table::SdrTableObj*>(m_xTableShape->GetSdrObject());

    bool bHBorder = false;
    bool bVBorder = false;
    bool bOutline = false;
    bool bKeys = false;

    std::vector<ViewLegendSymbol> aSymbols;

    m_xDataTableModel->getPropertyValue("HBorder") >>= bHBorder;
    m_xDataTableModel->getPropertyValue("VBorder") >>= bVBorder;
    m_xDataTableModel->getPropertyValue("Outline") >>= bOutline;
    m_xDataTableModel->getPropertyValue("Keys") >>= bKeys;

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

    // COLUMN HEADER

    nColumn = 1;
    for (auto const& rString : m_aXValues)
    {
        uno::Reference<table::XCell> xCell = xTable->getCellByPosition(nColumn, 0);
        uno::Reference<beans::XPropertySet> xPropertySet(xCell, uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCellTextRange(xCell, uno::UNO_QUERY);
        if (xCellTextRange.is())
        {
            xCellTextRange->setString(rString);

            bool bLeft
                = (bOutline && nColumn == 1) || (bVBorder && nColumn > 1 && nColumn < nColumnCount);
            bool bRight = (bOutline && nColumn == nColumnCount)
                          || (bVBorder && nColumn > 1 && nColumn < nColumnCount);
            setCellProperties(xPropertySet, bLeft, bOutline, bRight, bOutline);
            setCellCharAndParagraphProperties(xPropertySet);
        }
        nColumn++;
    }

    // ROW HEADER
    // Prepare keys
    if (bKeys)
    {
        awt::Size aMaxSymbolExtent(300, 300);
        for (VSeriesPlotter* pSeriesPlotter : m_pSeriesPlotterList)
        {
            if (pSeriesPlotter)
            {
                uno::Reference<lang::XMultiServiceFactory> xFactory(m_xChartModel, uno::UNO_QUERY);
                std::vector<ViewLegendSymbol> aNewEntries = pSeriesPlotter->createSymbols(
                    aMaxSymbolExtent, m_xTarget, xFactory, m_xComponentContext);
                aSymbols.insert(aSymbols.end(), aNewEntries.begin(), aNewEntries.end());
            }
        }
    }

    nRow = 1;
    for (auto const& rSeriesName : m_aDataSeriesNames)
    {
        uno::Reference<table::XCell> xCell = xTable->getCellByPosition(0, nRow);
        uno::Reference<beans::XPropertySet> xCellPropertySet(xCell, uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCellTextRange(xCell, uno::UNO_QUERY);
        if (xCellTextRange.is())
        {
            bool bTop = (bOutline && nRow == 1) || (bHBorder && nRow > 1 && nRow < nRowCount);
            bool bBottom
                = (bOutline && nRow == nRowCount) || (bHBorder && nRow > 1 && nRow < nRowCount);
            setCellProperties(xCellPropertySet, bOutline, bTop, bOutline, bBottom);

            auto xText = xCellTextRange->getText();
            xText->insertString(xText->getStart(), rSeriesName, false);
            uno::Reference<container::XEnumerationAccess> xEnumAccess(xText, uno::UNO_QUERY);
            uno::Reference<container::XEnumeration> xEnumeration(xEnumAccess->createEnumeration());
            uno::Reference<text::XTextRange> xParagraph(xEnumeration->nextElement(),
                                                        uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xTextPropertySet(xParagraph, uno::UNO_QUERY);

            setCellCharAndParagraphProperties(xTextPropertySet);

            xCellPropertySet->setPropertyValue("ParaAdjust", uno::Any(style::ParagraphAdjust_LEFT));
            if (bKeys)
                xCellPropertySet->setPropertyValue("ParaLeftMargin", uno::Any(sal_Int32(500)));
        }
        nRow++;
    }

    // TABLE
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

                setCellProperties(xPropertySet, bLeft, bTop, bRight, bBottom);
                setCellCharAndParagraphProperties(xPropertySet);
            }
            nColumn++;
        }
        nRow++;
    }

    xBroadcaster->unlockBroadcasts();

    pTableObject->DistributeColumns(0, nColumnCount - 1, true, true);
    pTableObject->DistributeRows(0, nRowCount - 1, true, true);

    uno::Reference<beans::XPropertySet> xPropertySet(xTableColumns->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nWidth = 0;
    xPropertySet->getPropertyValue("Width") >>= nWidth;

    sal_Int32 nTableX = basegfx::fround(rStart.getX() - nWidth);
    sal_Int32 nTableY = basegfx::fround(rStart.getY());
    m_xTableShape->setPosition({ nTableX, nTableY });

    for (sal_Int32 i = 1; i < xTableColumns->getCount(); ++i)
    {
        xPropertySet.set(xTableColumns->getByIndex(i), uno::UNO_QUERY);
        xPropertySet->setPropertyValue("Width", uno::Any(nColumnWidth));
    }

    if (bKeys)
    {
        sal_Int32 nTotalHeight = 0;
        for (sal_Int32 i = 0; i < xTableRows->getCount(); i++)
        {
            xPropertySet.set(xTableRows->getByIndex(i), uno::UNO_QUERY);
            sal_Int32 nHeight = 0;
            xPropertySet->getPropertyValue("Height") >>= nHeight;
            if (i > 0)
            {
                aSymbols[i - 1].aSymbol->setPosition(
                    { nTableX + 100, nTableY + nTotalHeight + 100 });
            }
            nTotalHeight += nHeight;
        }
    }
}

void DataTableView::initializeShapes(const css::uno::Reference<css::drawing::XShapes>& xTarget)
{
    m_xTarget = xTarget;
}

void DataTableView::initializeValues(
    std::vector<std::unique_ptr<VSeriesPlotter>>& rSeriesPlotterList)
{
    for (auto& rSeriesPlotter : rSeriesPlotterList)
    {
        m_pSeriesPlotterList.push_back(rSeriesPlotter.get());

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
