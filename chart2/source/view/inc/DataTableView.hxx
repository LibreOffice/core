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

#include <svx/unoshape.hxx>
#include <svx/unodraw/SvxTableShape.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <DataTable.hxx>
#include "VLineProperties.hxx"

namespace chart
{
class VSeriesPlotter;
class ChartModel;
class LegendEntryProvider;

class DataTableView final
{
private:
    rtl::Reference<::chart::ChartModel> m_xChartModel;
    rtl::Reference<SvxShapeGroupAnyD> m_xTarget;
    rtl::Reference<SvxTableShape> m_xTableShape;
    rtl::Reference<DataTable> m_xDataTableModel;
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;
    css::uno::Reference<css::table::XTable> m_xTable;
    VLineProperties m_aLineProperties;
    std::vector<VSeriesPlotter*> m_pSeriesPlotterList;

    std::vector<OUString> m_aDataSeriesNames;
    std::vector<OUString> m_aXValues;
    std::vector<std::vector<OUString>> m_pDataSeriesValues;
    bool m_bAlignAxisValuesWithColumns;

    void
    setCellCharAndParagraphProperties(css::uno::Reference<css::beans::XPropertySet>& xPropertySet);

    void setCellProperties(css::uno::Reference<css::beans::XPropertySet>& xPropertySet, bool bLeft,
                           bool bTop, bool bRight, bool bBottom);

public:
    DataTableView(rtl::Reference<::chart::ChartModel> const& xChartDoc,
                  rtl::Reference<DataTable> const& rDataTableModel,
                  css::uno::Reference<css::uno::XComponentContext> const& rComponentContext,
                  bool bAlignAxisValuesWithColumns);
    void initializeShapes(const rtl::Reference<SvxShapeGroupAnyD>& xTarget);
    void initializeValues(std::vector<std::unique_ptr<VSeriesPlotter>>& rSeriesPlotterList);
    void createShapes(basegfx::B2DVector const& rStart, basegfx::B2DVector const& rEnd,
                      sal_Int32 nAxisStepWidth);
    void changePosition(sal_Int32 x, sal_Int32 y);
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
