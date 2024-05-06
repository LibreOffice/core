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

#include "Chart2ModelContact.hxx"
#include <ChartModelHelper.hxx>
#include <Legend.hxx>
#include <LegendHelper.hxx>
#include <CommonConverters.hxx>
#include <servicenames.hxx>
#include <ObjectIdentifier.hxx>
#include <chartview/ExplicitValueProvider.hxx>
#include <chartview/DrawModelWrapper.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <ChartView.hxx>
#include <DiagramHelper.hxx>
#include <BaseCoordinateSystem.hxx>

#include <ChartModel.hxx>

#include <com/sun/star/chart2/XDataSeries.hpp>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;

namespace chart::wrapper
{

Chart2ModelContact::Chart2ModelContact(
    const Reference< uno::XComponentContext > & xContext ) :
        m_xContext( xContext ),
        m_xChartModel( nullptr )
{
}

Chart2ModelContact::~Chart2ModelContact()
{
    clear();
}

void Chart2ModelContact::setDocumentModel( ChartModel* pChartModel )
{
    clear();
    m_xChartModel = pChartModel;
    if( !pChartModel )
        return;

    m_aTableMap[u"LineDashName"_ustr].set(pChartModel->createInstance(u"com.sun.star.drawing.DashTable"_ustr), uno::UNO_QUERY);
    m_aTableMap[u"FillGradientName"_ustr].set(pChartModel->createInstance(u"com.sun.star.drawing.GradientTable"_ustr), uno::UNO_QUERY);
    m_aTableMap[u"FillHatchName"_ustr].set(pChartModel->createInstance(u"com.sun.star.drawing.HatchTable"_ustr), uno::UNO_QUERY);
    m_aTableMap[u"FillBitmapName"_ustr].set(pChartModel->createInstance(u"com.sun.star.drawing.BitmapTable"_ustr), uno::UNO_QUERY);
    m_aTableMap[u"FillTransparenceGradientName"_ustr].set(pChartModel->createInstance(u"com.sun.star.drawing.TransparencyGradientTable"_ustr), uno::UNO_QUERY);
}

void Chart2ModelContact::clear()
{
    m_xChartModel.clear();
    m_xChartView.clear();
}

rtl::Reference< ChartModel > Chart2ModelContact::getDocumentModel() const
{
    return m_xChartModel;
}

rtl::Reference< ::chart::Diagram > Chart2ModelContact::getDiagram() const
{
    try
    {
        rtl::Reference<ChartModel> xChartModel = getDocumentModel();
        if( xChartModel)
            return xChartModel->getFirstChartDiagram();
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return nullptr;
}

rtl::Reference< ::chart::ChartView > const & Chart2ModelContact::getChartView() const
{
    if(!m_xChartView.is())
    {
        // get the chart view
        rtl::Reference<ChartModel> xChartModel( m_xChartModel );
        if( xChartModel )
        {
            auto xInstance = xChartModel->createInstance( CHART_VIEW_SERVICE_NAME );
            auto pChartView = dynamic_cast<ChartView*>(xInstance.get());
            assert(!xInstance || pChartView);
            m_xChartView = pChartView;
        }
    }
    return m_xChartView;
}

ExplicitValueProvider* Chart2ModelContact::getExplicitValueProvider() const
{
    getChartView();

    //obtain the ExplicitValueProvider from the chart view
    return m_xChartView.get();
}

rtl::Reference<SvxDrawPage> Chart2ModelContact::getDrawPage() const
{
    rtl::Reference<SvxDrawPage> xResult;
    ExplicitValueProvider* pProvider( getExplicitValueProvider() );
    if( pProvider )
    {
        xResult = pProvider->getDrawModelWrapper()->getMainDrawPage();
    }
    return xResult;
}

void Chart2ModelContact::getExplicitValuesForAxis(
    const rtl::Reference< Axis > & xAxis,
    ExplicitScaleData &  rOutExplicitScale,
    ExplicitIncrementData & rOutExplicitIncrement )
{
    ExplicitValueProvider* pProvider( getExplicitValueProvider() );
    if( pProvider )
    {
        pProvider->getExplicitValuesForAxis(
            xAxis, rOutExplicitScale, rOutExplicitIncrement );
    }
}

sal_Int32 Chart2ModelContact::getExplicitNumberFormatKeyForAxis(
            const rtl::Reference< ::chart::Axis >& xAxis )
{
    rtl::Reference< BaseCoordinateSystem > xCooSys(
        AxisHelper::getCoordinateSystemOfAxis(
              xAxis, m_xChartModel.get()->getFirstChartDiagram() ) );

    return ExplicitValueProvider::getExplicitNumberFormatKeyForAxis( xAxis, xCooSys
              , m_xChartModel.get() );
}

sal_Int32 Chart2ModelContact::getExplicitNumberFormatKeyForSeries(
            const Reference< chart2::XDataSeries >& xSeries )
{
    return ExplicitValueProvider::getExplicitNumberFormatKeyForDataLabel(
        uno::Reference< beans::XPropertySet >( xSeries, uno::UNO_QUERY ));
}

awt::Size Chart2ModelContact::GetPageSize() const
{
    return ChartModelHelper::getPageSize(m_xChartModel.get());
}

awt::Rectangle Chart2ModelContact::SubstractAxisTitleSizes( const awt::Rectangle& rPositionRect )
{
    awt::Rectangle aRect = ExplicitValueProvider::AddSubtractAxisTitleSizes(
        *m_xChartModel.get(), getChartView().get(), rPositionRect, true );
    return aRect;
}

awt::Rectangle Chart2ModelContact::GetDiagramRectangleIncludingTitle() const
{
    awt::Rectangle aRect( GetDiagramRectangleIncludingAxes() );

    //add axis title sizes to the diagram size
    aRect = ExplicitValueProvider::AddSubtractAxisTitleSizes(
        *m_xChartModel.get(), getChartView().get(), aRect, false );

    return aRect;
}

awt::Rectangle Chart2ModelContact::GetDiagramRectangleIncludingAxes() const
{
    awt::Rectangle aRect(0,0,0,0);
    rtl::Reference< Diagram > xDiagram = m_xChartModel.get()->getFirstChartDiagram();

    if( xDiagram && xDiagram->getDiagramPositioningMode() == DiagramPositioningMode::Including )
        aRect = DiagramHelper::getDiagramRectangleFromModel(m_xChartModel.get());
    else
    {
        ExplicitValueProvider* pProvider( getExplicitValueProvider() );
        if( pProvider )
            aRect = pProvider->getRectangleOfObject(u"PlotAreaIncludingAxes"_ustr);
    }
    return aRect;
}

awt::Rectangle Chart2ModelContact::GetDiagramRectangleExcludingAxes() const
{
    awt::Rectangle aRect(0,0,0,0);
    rtl::Reference< Diagram > xDiagram = m_xChartModel.get()->getFirstChartDiagram();

    if( xDiagram && xDiagram->getDiagramPositioningMode() == DiagramPositioningMode::Excluding )
        aRect = DiagramHelper::getDiagramRectangleFromModel(m_xChartModel.get());
    else
    {
        ExplicitValueProvider* pProvider( getExplicitValueProvider() );
        if( pProvider )
            aRect = pProvider->getDiagramRectangleExcludingAxes();
    }
    return aRect;
}

awt::Size Chart2ModelContact::GetLegendSize() const
{
    awt::Size aSize;
    ExplicitValueProvider* pProvider( getExplicitValueProvider() );
    if( pProvider )
    {
        rtl::Reference< Legend > xLegend = LegendHelper::getLegend( *m_xChartModel.get() );
        OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xLegend, m_xChartModel ) );
        aSize = ToSize( pProvider->getRectangleOfObject( aCID ) );
    }
    return aSize;
}

awt::Point Chart2ModelContact::GetLegendPosition() const
{
    awt::Point aPoint;
    ExplicitValueProvider* pProvider( getExplicitValueProvider() );
    if( pProvider )
    {
        rtl::Reference< Legend > xLegend = LegendHelper::getLegend( *m_xChartModel.get() );
        OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xLegend, m_xChartModel ) );
        aPoint = ToPoint( pProvider->getRectangleOfObject( aCID ) );
    }
    return aPoint;
}

awt::Size Chart2ModelContact::GetTitleSize( const uno::Reference< css::chart2::XTitle > & xTitle ) const
{
    awt::Size aSize;
    ExplicitValueProvider* pProvider( getExplicitValueProvider() );
    if( pProvider && xTitle.is() )
    {
        OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xTitle, m_xChartModel ) );
        aSize = ToSize( pProvider->getRectangleOfObject( aCID ) );
    }
    return aSize;
}

awt::Point Chart2ModelContact::GetTitlePosition( const uno::Reference< css::chart2::XTitle > & xTitle ) const
{
    awt::Point aPoint;
    ExplicitValueProvider* pProvider( getExplicitValueProvider() );
    if( pProvider && xTitle.is() )
    {
        OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xTitle, m_xChartModel.get() ) );
        aPoint = ToPoint( pProvider->getRectangleOfObject( aCID ) );
    }
    return aPoint;
}

awt::Size Chart2ModelContact::GetAxisSize( const uno::Reference< css::chart2::XAxis > & xAxis ) const
{
    awt::Size aSize;
    ExplicitValueProvider* pProvider( getExplicitValueProvider() );
    if( pProvider && xAxis.is() )
    {
        OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xAxis, m_xChartModel.get() ) );
        aSize = ToSize( pProvider->getRectangleOfObject( aCID ) );
    }
    return aSize;
}

awt::Point Chart2ModelContact::GetAxisPosition( const uno::Reference< css::chart2::XAxis > & xAxis ) const
{
    awt::Point aPoint;
    ExplicitValueProvider* pProvider( getExplicitValueProvider() );
    if( pProvider && xAxis.is() )
    {
        OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xAxis, m_xChartModel.get() ) );
        aPoint = ToPoint( pProvider->getRectangleOfObject( aCID ) );
    }
    return aPoint;
}

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
