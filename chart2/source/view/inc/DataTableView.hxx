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

#include <svx/unodraw/SvxTableShape.hxx>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <DataTable.hxx>
#include "VLineProperties.hxx"

namespace chart
{
class VSeriesPlotter;

class DataTableView final
{
private:
    css::uno::Reference<css::chart2::XChartDocument> m_xChartModel;
    css::uno::Reference<css::drawing::XShapes> m_xTarget;
    rtl::Reference<SvxTableShape> m_xTableShape;
    rtl::Reference<DataTable> m_xDataTableModel;
    css::uno::Reference<css::lang::XMultiServiceFactory> m_xShapeFactory;
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;
    VLineProperties m_aLineProperties;
    std::vector<VSeriesPlotter*> m_pSeriesPlotterList;

    std::vector<OUString> m_aDataSeriesNames;
    std::vector<OUString> m_aXValues;
    std::vector<std::vector<OUString>> m_pDataSeriesValues;

    void
    setCellCharAndParagraphProperties(css::uno::Reference<css::beans::XPropertySet>& xPropertySet);

    void setCellProperties(css::uno::Reference<css::beans::XPropertySet>& xPropertySet, bool bLeft,
                           bool bTop, bool bRight, bool bBottom);

public:
    DataTableView(css::uno::Reference<css::chart2::XChartDocument> const& xChartDoc,
                  rtl::Reference<DataTable> const& rDataTableModel,
                  css::uno::Reference<css::uno::XComponentContext> const& rComponentContext);
    void initializeShapes(const css::uno::Reference<css::drawing::XShapes>& xTarget);
    void initializeValues(std::vector<std::unique_ptr<VSeriesPlotter>>& rSeriesPlotterList);
    void createShapes(basegfx::B2DVector const& rStart, basegfx::B2DVector const& rEnd,
                      sal_Int32 nColumnWidth);
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
