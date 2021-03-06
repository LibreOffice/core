/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/XAxisXSupplier.hpp>
#include <com/sun/star/chart/XAxisYSupplier.hpp>
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisXSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisYSupplier.hpp>
#include <ooo/vba/excel/XChart.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl<ov::excel::XChart > ChartImpl_BASE;

class ScVbaChart : public ChartImpl_BASE
{
friend class ScVbaAxis;

    css::uno::Reference< css::chart::XChartDocument > mxChartDocument;
    css::uno::Reference< css::table::XTableChart > mxTableChart;
    css::uno::Reference< css::beans::XPropertySet > mxDiagramPropertySet;
    css::uno::Reference< css::beans::XPropertySet > mxChartPropertySet;
    css::uno::Reference< css::chart::XAxisXSupplier > xAxisXSupplier;
    css::uno::Reference< css::chart::XAxisYSupplier> xAxisYSupplier;
    css::uno::Reference< css::chart::XAxisZSupplier > xAxisZSupplier;
    css::uno::Reference< css::chart::XTwoAxisXSupplier > xTwoAxisXSupplier;
    css::uno::Reference< css::chart::XTwoAxisYSupplier > xTwoAxisYSupplier;

    static css::uno::Sequence< OUString > getDefaultSeriesDescriptions( sal_Int32 nCount );
    /// @throws css::script::BasicErrorException
    void setDefaultChartType() ;
    /// @throws css::script::BasicErrorException
    void setDiagram( const OUString& _sDiagramType);
    /// @throws css::uno::RuntimeException
    bool isStacked();
    /// @throws css::uno::RuntimeException
    bool is100PercentStacked();
    /// @throws css::uno::RuntimeException
    sal_Int32 getStackedType( sal_Int32 _nStacked, sal_Int32 _n100PercentStacked, sal_Int32 _nUnStacked );
    /// @throws css::script::BasicErrorException
    sal_Int32 getSolidType(sal_Int32 _nDeep, sal_Int32 _nVertiStacked, sal_Int32 _nVerti100PercentStacked, sal_Int32 _nVertiUnStacked, sal_Int32 _nHoriStacked, sal_Int32 _nHori100PercentStacked, sal_Int32 _nHoriUnStacked);
    /// @throws css::script::BasicErrorException
    sal_Int32 getStockUpDownValue(sal_Int32 _nUpDown, sal_Int32 _nNotUpDown);
    /// @throws css::script::BasicErrorException
    bool hasMarkers();
    /// @throws css::script::BasicErrorException
    sal_Int32 getMarkerType(sal_Int32 _nWithMarkers, sal_Int32 _nWithoutMarkers);
    void assignDiagramAttributes();
public:
    ScVbaChart( const css::uno::Reference< ov::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext, const css::uno::Reference< css::lang::XComponent >& _xChartComponent, const css::uno::Reference< css::table::XTableChart >& _xTableChart );

    // Non-interface
    const css::uno::Reference< css::beans::XPropertySet >& xDiagramPropertySet() const { return mxDiagramPropertySet; }
    /// @throws css::uno::RuntimeException
    bool is3D();
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::beans::XPropertySet > getAxisPropertySet(sal_Int32 _nAxisType, sal_Int32 _nAxisGroup);
    // Methods
    virtual OUString SAL_CALL getName() override;
    virtual css::uno::Any SAL_CALL SeriesCollection(const css::uno::Any&) override;
    virtual ::sal_Int32 SAL_CALL getChartType() override;
    virtual void SAL_CALL setChartType( ::sal_Int32 _charttype ) override;
    virtual void SAL_CALL Activate(  ) override;
    virtual void SAL_CALL setSourceData( const css::uno::Reference< ::ooo::vba::excel::XRange >& range, const css::uno::Any& PlotBy ) override;
    virtual ::sal_Int32 SAL_CALL Location(  ) override;
    virtual ::sal_Int32 SAL_CALL getLocation(  ) override;
    virtual void SAL_CALL setLocation( ::sal_Int32 where, const css::uno::Any& Name ) override;
    virtual sal_Bool SAL_CALL getHasTitle(  ) override;
    virtual void SAL_CALL setHasTitle( sal_Bool bTitle ) override;
    virtual sal_Bool SAL_CALL getHasLegend(  ) override;
    virtual void SAL_CALL setHasLegend( sal_Bool bLegend ) override;
    virtual void SAL_CALL setPlotBy( ::sal_Int32 xlRowCol ) override;
    virtual ::sal_Int32 SAL_CALL getPlotBy(  ) override;
    virtual css::uno::Reference< ov::excel::XChartTitle > SAL_CALL getChartTitle(  ) override;
    virtual css::uno::Any SAL_CALL Axes( const css::uno::Any& Type, const css::uno::Any& AxisGroup ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
