/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <cpo/uno/XComponentContext.hpp>
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

    cpo::uno::Reference< css::chart::XChartDocument > mxChartDocument;
    cpo::uno::Reference< css::table::XTableChart > mxTableChart;
    cpo::uno::Reference< css::beans::XPropertySet > mxDiagramPropertySet;
    cpo::uno::Reference< css::beans::XPropertySet > mxChartPropertySet;
    cpo::uno::Reference< css::chart::XAxisXSupplier > xAxisXSupplier;
    cpo::uno::Reference< css::chart::XAxisYSupplier> xAxisYSupplier;
    cpo::uno::Reference< css::chart::XAxisZSupplier > xAxisZSupplier;
    cpo::uno::Reference< css::chart::XTwoAxisXSupplier > xTwoAxisXSupplier;
    cpo::uno::Reference< css::chart::XTwoAxisYSupplier > xTwoAxisYSupplier;

    static cpo::uno::Sequence< OUString > getDefaultSeriesDescriptions( sal_Int32 nCount );
    /// @throws css::script::BasicErrorException
    void setDefaultChartType() ;
    /// @throws css::script::BasicErrorException
    void setDiagram( const OUString& _sDiagramType);
    /// @throws cpo::uno::RuntimeException
    bool isStacked();
    /// @throws cpo::uno::RuntimeException
    bool is100PercentStacked();
    /// @throws cpo::uno::RuntimeException
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
    ScVbaChart( const cpo::uno::Reference< ov::XHelperInterface >& _xParent, const cpo::uno::Reference< cpo::uno::XComponentContext >& _xContext, const cpo::uno::Reference< css::lang::XComponent >& _xChartComponent, cpo::uno::Reference< css::table::XTableChart >  _xTableChart );

    // Non-interface
    const cpo::uno::Reference< css::beans::XPropertySet >& xDiagramPropertySet() const { return mxDiagramPropertySet; }
    /// @throws cpo::uno::RuntimeException
    bool is3D();
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    cpo::uno::Reference< css::beans::XPropertySet > getAxisPropertySet(sal_Int32 _nAxisType, sal_Int32 _nAxisGroup);
    // Methods
    virtual OUString getName() override;
    virtual cpo::uno::Any SeriesCollection(const cpo::uno::Any&) override;
    virtual ::sal_Int32 getChartType() override;
    virtual void setChartType( ::sal_Int32 _charttype ) override;
    virtual void Activate(  ) override;
    virtual void setSourceData( const cpo::uno::Reference< ::ooo::vba::excel::XRange >& range, const cpo::uno::Any& PlotBy ) override;
    virtual ::sal_Int32 Location(  ) override;
    virtual ::sal_Int32 getLocation(  ) override;
    virtual void setLocation( ::sal_Int32 where, const cpo::uno::Any& Name ) override;
    virtual bool getHasTitle(  ) override;
    virtual void setHasTitle( bool bTitle ) override;
    virtual bool getHasLegend(  ) override;
    virtual void setHasLegend( bool bLegend ) override;
    virtual void setPlotBy( ::sal_Int32 xlRowCol ) override;
    virtual ::sal_Int32 getPlotBy(  ) override;
    virtual cpo::uno::Reference< ov::excel::XChartTitle > getChartTitle(  ) override;
    virtual cpo::uno::Any Axes( const cpo::uno::Any& Type, const cpo::uno::Any& AxisGroup ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
