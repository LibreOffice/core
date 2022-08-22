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

#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
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

uno::Reference<text::XTextRange> getFirstParagraph(uno::Reference<text::XText> const& xText)
{
    uno::Reference<text::XTextRange> xParagraph;
    uno::Reference<container::XEnumerationAccess> xEnumAccess(xText, uno::UNO_QUERY);
    if (!xEnumAccess.is())
        return xParagraph;
    uno::Reference<container::XEnumeration> xEnumeration(xEnumAccess->createEnumeration());
    xParagraph.set(xEnumeration->nextElement(), uno::UNO_QUERY);
    return xParagraph;
}

uno::Reference<beans::XPropertySet>
getFirstParagraphProperties(uno::Reference<text::XText> const& xText)
{
    uno::Reference<beans::XPropertySet> xPropertySet;
    auto xParagraph = getFirstParagraph(xText);
    if (!xParagraph.is())
        return xPropertySet;
    xPropertySet.set(xParagraph, uno::UNO_QUERY);
    return xPropertySet;
}

} // end anonymous namespace

DataTableView::DataTableView(
    rtl::Reference<::chart::ChartModel> const& xChartModel,
    rtl::Reference<DataTable> const& rDataTableModel,
    css::uno::Reference<css::uno::XComponentContext> const& rComponentContext,
    bool bAlignAxisValuesWithColumns)
    : m_xChartModel(xChartModel)
    , m_xDataTableModel(rDataTableModel)
    , m_xComponentContext(rComponentContext)
    , m_bAlignAxisValuesWithColumns(bAlignAxisValuesWithColumns)
{
    uno::Reference<beans::XPropertySet> xPropertySet(m_xDataTableModel);
    m_aLineProperties.initFromPropertySet(xPropertySet);
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

    drawing::FillStyle eFillStyle = drawing::FillStyle_NONE;
    xDataTableProperties->getPropertyValue("FillStyle") >>= eFillStyle;
    if (eFillStyle == drawing::FillStyle_SOLID)
    {
        sal_Int32 aColor = 0;
        if (xDataTableProperties->getPropertyValue("FillColor") >>= aColor)
            xPropertySet->setPropertyValue("CharBackColor", uno::Any(aColor));
    }

    xPropertySet->setPropertyValue("ParaAdjust", uno::Any(style::ParagraphAdjust_CENTER));
}

void DataTableView::setCellProperties(css::uno::Reference<beans::XPropertySet>& xPropertySet,
                                      bool bLeft, bool bTop, bool bRight, bool bBottom)
{
    xPropertySet->setPropertyValue("FillColor", uno::Any(Color(0xFFFFFF)));

    uno::Reference<beans::XPropertySet> xDataTableProperties(m_xDataTableModel);
    float fFontHeight = 0.0;
    xDataTableProperties->getPropertyValue("CharHeight") >>= fFontHeight;
    fFontHeight = o3tl::convert(fFontHeight, o3tl::Length::pt, o3tl::Length::mm100);
    sal_Int32 nXDistance = std::round(fFontHeight * 0.18f);
    sal_Int32 nYDistance = std::round(fFontHeight * 0.30f);

    xPropertySet->setPropertyValue("TextLeftDistance", uno::Any(nXDistance));
    xPropertySet->setPropertyValue("TextRightDistance", uno::Any(nXDistance));
    xPropertySet->setPropertyValue("TextUpperDistance", uno::Any(nYDistance));
    xPropertySet->setPropertyValue("TextLowerDistance", uno::Any(nYDistance));

    xPropertySet->setPropertyValue("TextVerticalAdjust", uno::Any(drawing::TextVerticalAdjust_TOP));

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
                                 sal_Int32 nAxisStepWidth)
{
    if (!m_xTarget.is())
        return;

    ShapeFactory::removeSubShapes(m_xTarget);
    auto sParticle = ObjectIdentifier::createParticleForDataTable(m_xChartModel);
    auto sCID = ObjectIdentifier::createClassifiedIdentifierForParticle(sParticle);
    m_xTableShape = ShapeFactory::createTable(m_xTarget, sCID);

    auto rDelta = rEnd - rStart;
    sal_Int32 nTableSize = basegfx::fround(rDelta.getX());
    m_xTableShape->setSize({ nTableSize, 0 });

    try
    {
        m_xTableShape->getPropertyValue("Model") >>= m_xTable;
    }
    catch (const uno::Exception&)
    {
        return;
    }

    if (!m_xTable.is())
        return;

    uno::Reference<util::XBroadcaster> xBroadcaster(m_xTable, uno::UNO_QUERY);

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
    uno::Reference<table::XTableColumns> xTableColumns = m_xTable->getColumns();
    xTableColumns->insertByIndex(0, nColumnCount);

    sal_Int32 nRowCount = m_aDataSeriesNames.size();
    uno::Reference<table::XTableRows> xTableRows = m_xTable->getRows();
    xTableRows->insertByIndex(0, nRowCount);

    sal_Int32 nColumnWidth = 0.0;

    // If we don't align, we have to calculate the column width ourselves
    if (m_bAlignAxisValuesWithColumns)
        nColumnWidth = nAxisStepWidth;
    else
        nColumnWidth = double(nTableSize) / nColumnCount;

    // Setup empty top-left cell
    {
        uno::Reference<table::XCell> xCell = m_xTable->getCellByPosition(0, 0);
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
        uno::Reference<table::XCell> xCell = m_xTable->getCellByPosition(nColumn, 0);
        uno::Reference<beans::XPropertySet> xPropertySet(xCell, uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCellTextRange(xCell, uno::UNO_QUERY);
        if (xCellTextRange.is())
        {
            auto xText = xCellTextRange->getText();
            xText->insertString(xText->getStart(), rString, false);
            auto xTextPropertySet = getFirstParagraphProperties(xText);
            if (!xTextPropertySet.is())
                continue;

            bool bLeft
                = (bOutline && nColumn == 1) || (bVBorder && nColumn > 1 && nColumn < nColumnCount);
            bool bRight = (bOutline && nColumn == nColumnCount)
                          || (bVBorder && nColumn > 1 && nColumn < nColumnCount);
            setCellCharAndParagraphProperties(xTextPropertySet);
            setCellProperties(xPropertySet, bLeft, bOutline, bRight, bOutline);
        }
        nColumn++;
    }

    // ROW HEADER
    // Prepare keys (symbols)
    sal_Int32 nMaxSymbolWidth = 0;
    constexpr const sal_Int32 constSymbolMargin = 100; // 1mm
    if (bKeys)
    {
        uno::Reference<beans::XPropertySet> xDataTableProperties(m_xDataTableModel);
        float fFontHeight = 0.0;
        xDataTableProperties->getPropertyValue("CharHeight") >>= fFontHeight;
        fFontHeight = o3tl::convert(fFontHeight, o3tl::Length::pt, o3tl::Length::mm100);

        sal_Int32 nSymbolHeight = sal_Int32(fFontHeight * 0.6);
        sal_Int32 nSymbolWidth = nSymbolHeight;

        for (VSeriesPlotter* pSeriesPlotter : m_pSeriesPlotterList)
        {
            if (pSeriesPlotter)
            {
                awt::Size aCurrentRatio = pSeriesPlotter->getPreferredLegendKeyAspectRatio();
                sal_Int32 nCurrentWidth = aCurrentRatio.Width;
                if (aCurrentRatio.Height > 0)
                    nCurrentWidth = nSymbolHeight * aCurrentRatio.Width / aCurrentRatio.Height;
                nSymbolWidth = std::max(nSymbolWidth, nCurrentWidth);
            }
        }
        nMaxSymbolWidth = nSymbolWidth;

        for (VSeriesPlotter* pSeriesPlotter : m_pSeriesPlotterList)
        {
            if (pSeriesPlotter)
            {
                awt::Size aSize(nSymbolWidth, nSymbolHeight);
                std::vector<ViewLegendSymbol> aNewEntries
                    = pSeriesPlotter->createSymbols(aSize, m_xTarget, m_xComponentContext);

                for (auto const& rSymbol : aNewEntries)
                    aSymbols.push_back(rSymbol);
            }
        }
    }

    nRow = 1;
    for (auto const& rSeriesName : m_aDataSeriesNames)
    {
        uno::Reference<table::XCell> xCell = m_xTable->getCellByPosition(0, nRow);
        uno::Reference<beans::XPropertySet> xCellPropertySet(xCell, uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCellTextRange(xCell, uno::UNO_QUERY);
        if (xCellTextRange.is())
        {
            bool bTop = (bOutline && nRow == 1) || (bHBorder && nRow > 1 && nRow < nRowCount);
            bool bBottom
                = (bOutline && nRow == nRowCount) || (bHBorder && nRow > 1 && nRow < nRowCount);

            auto xText = xCellTextRange->getText();
            xText->insertString(xText->getStart(), rSeriesName, false);
            auto xTextPropertySet = getFirstParagraphProperties(xText);
            if (!xTextPropertySet.is())
                continue;
            setCellCharAndParagraphProperties(xTextPropertySet);
            setCellProperties(xCellPropertySet, bOutline, bTop, bOutline, bBottom);

            xCellPropertySet->setPropertyValue("ParaAdjust", uno::Any(style::ParagraphAdjust_LEFT));
            if (bKeys)
            {
                xCellPropertySet->setPropertyValue(
                    "ParaLeftMargin", uno::Any(nMaxSymbolWidth + sal_Int32(2 * constSymbolMargin)));
            }
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
            uno::Reference<table::XCell> xCell = m_xTable->getCellByPosition(nColumn, nRow);
            uno::Reference<beans::XPropertySet> xCellPropertySet(xCell, uno::UNO_QUERY);
            uno::Reference<text::XTextRange> xCellTextRange(xCell, uno::UNO_QUERY);
            if (xCellTextRange.is())
            {
                auto xText = xCellTextRange->getText();
                xText->insertString(xText->getStart(), rValue, false);
                auto xTextPropertySet = getFirstParagraphProperties(xText);
                if (!xTextPropertySet.is())
                    continue;

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

                setCellCharAndParagraphProperties(xTextPropertySet);
                setCellProperties(xCellPropertySet, bLeft, bTop, bRight, bBottom);
            }
            nColumn++;
        }
        nRow++;
    }

    xBroadcaster->unlockBroadcasts();

    pTableObject->DistributeColumns(0, nColumnCount, true, true);
    pTableObject->DistributeRows(0, nRowCount, true, true);

    xBroadcaster->lockBroadcasts();

    changePosition(basegfx::fround(rStart.getX()), basegfx::fround(rStart.getY()));

    sal_Int32 nTableX = m_xTableShape->getPosition().X;
    sal_Int32 nTableY = m_xTableShape->getPosition().Y;

    uno::Reference<beans::XPropertySet> xPropertySet(xTableColumns->getByIndex(0), uno::UNO_QUERY);

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
            sal_Int32 nSymbolIndex = i - 1;
            if (nSymbolIndex < sal_Int32(aSymbols.size()))
            {
                xPropertySet.set(xTableRows->getByIndex(i), uno::UNO_QUERY);
                sal_Int32 nHeight = 0;
                xPropertySet->getPropertyValue("Height") >>= nHeight;
                if (i > 0)
                {
                    auto& rSymbol = aSymbols[nSymbolIndex].xSymbol;
                    sal_Int32 nSymbolHeight = rSymbol->getSize().Height;
                    sal_Int32 nSymbolY
                        = basegfx::fround(double(nHeight) / 2.0 - double(nSymbolHeight) / 2.0);
                    rSymbol->setPosition(
                        { nTableX + constSymbolMargin, nTableY + nTotalHeight + nSymbolY });
                }
                nTotalHeight += nHeight;
            }
        }
    }
    xBroadcaster->unlockBroadcasts();
}

void DataTableView::changePosition(sal_Int32 x, sal_Int32 y)
{
    if (!m_xTable.is())
        return;

    uno::Reference<table::XTableColumns> xTableColumns = m_xTable->getColumns();
    uno::Reference<beans::XPropertySet> xPropertySet(xTableColumns->getByIndex(0), uno::UNO_QUERY);

    sal_Int32 nWidth = 0;
    xPropertySet->getPropertyValue("Width") >>= nWidth;

    m_xTarget->setPosition({ x - nWidth, y });
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
