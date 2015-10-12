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

#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBACHART_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBACHART_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/XAxisXSupplier.hpp>
#include <com/sun/star/chart/XAxisYSupplier.hpp>
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisXSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisYSupplier.hpp>
#include <ooo/vba/excel/XChart.hpp>
#include <ooo/vba/excel/XDataLabels.hpp>
#include <ooo/vba/excel/XSeries.hpp>
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
    css::uno::Sequence< css::uno::Sequence< double > > dblValues;
    void setDefaultChartType()throw ( css::script::BasicErrorException ) ;
    void setDiagram( const OUString& _sDiagramType) throw( css::script::BasicErrorException );
    bool isStacked() throw ( css::uno::RuntimeException );
    bool is100PercentStacked() throw ( css::uno::RuntimeException );
    sal_Int32 getStackedType( sal_Int32 _nStacked, sal_Int32 _n100PercentStacked, sal_Int32 _nUnStacked ) throw ( css::uno::RuntimeException );
    sal_Int32 getSolidType(sal_Int32 _nDeep, sal_Int32 _nVertiStacked, sal_Int32 _nVerti100PercentStacked, sal_Int32 _nVertiUnStacked, sal_Int32 _nHoriStacked, sal_Int32 _nHori100PercentStacked, sal_Int32 _nHoriUnStacked) throw ( css::script::BasicErrorException );
    sal_Int32 getStockUpDownValue(sal_Int32 _nUpDown, sal_Int32 _nNotUpDown) throw (css::script::BasicErrorException);
    bool hasMarkers() throw ( css::script::BasicErrorException );
    sal_Int32 getMarkerType(sal_Int32 _nWithMarkers, sal_Int32 _nWithoutMarkers) throw ( css::script::BasicErrorException );
    void assignDiagramAttributes();
public:
    ScVbaChart( const css::uno::Reference< ov::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext, const css::uno::Reference< css::lang::XComponent >& _xChartComponent, const css::uno::Reference< css::table::XTableChart >& _xTableChart );

    // Non-interface
    css::uno::Reference< css::beans::XPropertySet > xDiagramPropertySet() const { return mxDiagramPropertySet; }
    bool is3D() throw ( css::uno::RuntimeException );
    css::uno::Reference< css::beans::XPropertySet > getAxisPropertySet(sal_Int32 _nAxisType, sal_Int32 _nAxisGroup) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    // Methods
    virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL SeriesCollection(const css::uno::Any&) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getChartType() throw ( css::uno::RuntimeException, css::script::BasicErrorException, std::exception) override;
    virtual void SAL_CALL setChartType( ::sal_Int32 _charttype ) throw ( css::uno::RuntimeException, css::script::BasicErrorException, std::exception) override;
    virtual void SAL_CALL Activate(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSourceData( const css::uno::Reference< ::ooo::vba::excel::XRange >& range, const css::uno::Any& PlotBy ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL Location(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getLocation(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLocation( ::sal_Int32 where, const css::uno::Any& Name ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getHasTitle(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHasTitle( sal_Bool bTitle ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getHasLegend(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHasLegend( sal_Bool bLegend ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPlotBy( ::sal_Int32 xlRowCol ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getPlotBy(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XChartTitle > SAL_CALL getChartTitle(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Axes( const css::uno::Any& Type, const css::uno::Any& AxisGroup ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

};

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBACHART_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
