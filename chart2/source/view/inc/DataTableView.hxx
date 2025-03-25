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

/**
 * DataTableView is responsible to create the table object, set the cell
 * properties accordingly to the model and fill it with the chart series
 * data.
 */
class DataTableView final
{
private:
    rtl::Reference<::chart::ChartModel> m_xChartModel;
    // the target shape
    rtl::Reference<SvxShapeGroupAnyD> m_xTarget;
    // the data table shape
    rtl::Reference<SvxTableShape> m_xTableShape;
    // the data table model
    rtl::Reference<DataTable> m_xDataTableModel;
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;
    css::uno::Reference<css::table::XTable> m_xTable;
    VLineProperties m_aLineProperties;
    std::vector<VSeriesPlotter*> m_pSeriesPlotterList;

    // data series names
    std::vector<OUString> m_aDataSeriesNames;
    // X axis names
    std::vector<OUString> m_aXValues;
    // list of data series values
    std::vector<std::vector<OUString>> m_pDataSeriesValues;

    // if the header vales should be aligned with the x-axis vales
    bool m_bAlignAxisValuesWithColumns;

    /** Set the char and paragraph properties for the input (value) cell */
    void setCellCharAndParagraphProperties(
        const css::uno::Reference<css::beans::XPropertySet>& xPropertySet);

    /** Set the common cell properties (for all cells in the data table,
     *  including headers)
     */
    void setCellProperties(const css::uno::Reference<css::beans::XPropertySet>& xPropertySet,
                           bool bLeft, bool bTop, bool bRight, bool bBottom);

public:
    DataTableView(rtl::Reference<::chart::ChartModel> const& xChartDoc,
                  rtl::Reference<DataTable> const& rDataTableModel,
                  css::uno::Reference<css::uno::XComponentContext> const& rComponentContext,
                  bool bAlignAxisValuesWithColumns);

    /** Initializes and prepares the target and data table shape */
    void initializeShapes(const rtl::Reference<SvxShapeGroupAnyD>& xTarget);

    /** Prepares the values of the chart, which will be shown it the data table */
    void initializeValues(std::vector<std::unique_ptr<VSeriesPlotter>>& rSeriesPlotterList);

    /** Creates the data table and fills the values */
    void createShapes(basegfx::B2DVector const& rStart, basegfx::B2DVector const& rEnd,
                      sal_Int32 nAxisStepWidth);

    /** Repositions the data table shape */
    void changePosition(sal_Int32 x, sal_Int32 y);
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
