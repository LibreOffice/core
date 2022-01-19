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
#include <LegendHelper.hxx>
#include <CommonConverters.hxx>
#include <servicenames.hxx>
#include <ObjectIdentifier.hxx>
#include <chartview/ExplicitValueProvider.hxx>
#include <chartview/DrawModelWrapper.hxx>
#include <AxisHelper.hxx>
#include <ChartView.hxx>
#include <DiagramHelper.hxx>

#include <ChartModel.hxx>

#include <comphelper/servicehelper.hxx>
#include <tools/diagnose_ex.h>

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

    uno::Reference< container::XNameContainer > xDashTable( pChartModel->createInstance("com.sun.star.drawing.DashTable"), uno::UNO_QUERY );
    uno::Reference< container::XNameContainer > xGradientTable( pChartModel->createInstance("com.sun.star.drawing.GradientTable"), uno::UNO_QUERY );
    uno::Reference< container::XNameContainer > xHatchTable( pChartModel->createInstance("com.sun.star.drawing.HatchTable"), uno::UNO_QUERY );
    uno::Reference< container::XNameContainer > xBitmapTable( pChartModel->createInstance("com.sun.star.drawing.BitmapTable"), uno::UNO_QUERY );
    uno::Reference< container::XNameContainer > xTransparencyGradientTable( pChartModel->createInstance("com.sun.star.drawing.TransparencyGradientTable"), uno::UNO_QUERY );
    m_aTableMap["LineDashName"] = xDashTable;
    m_aTableMap["FillGradientName"] = xGradientTable;
    m_aTableMap["FillHatchName"] = xHatchTable;
    m_aTableMap["FillBitmapName"] = xBitmapTable;
    m_aTableMap["FillTransparenceGradientName"] = xTransparencyGradientTable;
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
            m_xChartView = xChartModel->getChartView(); // will create if necessary
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
    const Reference< XAxis > & xAxis,
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
            const Reference< chart2::XAxis >& xAxis )
{
    Reference< chart2::XCoordinateSystem > xCooSys(
        AxisHelper::getCoordinateSystemOfAxis(
              xAxis, ChartModelHelper::findDiagram( uno::Reference<XChartDocument>(m_xChartModel.get()) ) ) );

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
    rtl::Reference< Diagram > xDiagram( ChartModelHelper::findDiagram( uno::Reference<XChartDocument>(m_xChartModel.get()) ) );

    if( DiagramHelper::getDiagramPositioningMode( xDiagram ) == DiagramPositioningMode_INCLUDING )
        aRect = DiagramHelper::getDiagramRectangleFromModel(m_xChartModel.get());
    else
    {
        ExplicitValueProvider* pProvider( getExplicitValueProvider() );
        if( pProvider )
            aRect = pProvider->getRectangleOfObject("PlotAreaIncludingAxes");
    }
    return aRect;
}

awt::Rectangle Chart2ModelContact::GetDiagramRectangleExcludingAxes() const
{
    awt::Rectangle aRect(0,0,0,0);
    rtl::Reference< Diagram > xDiagram( ChartModelHelper::findDiagram( uno::Reference<XChartDocument>(m_xChartModel.get()) ) );

    if( DiagramHelper::getDiagramPositioningMode( xDiagram ) == DiagramPositioningMode_EXCLUDING )
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
        uno::Reference< chart2::XLegend > xLegend( LegendHelper::getLegend( *m_xChartModel.get() ) );
        OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xLegend, uno::Reference<XChartDocument>(m_xChartModel.get()) ) );
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
        uno::Reference< chart2::XLegend > xLegend( LegendHelper::getLegend( *m_xChartModel.get() ) );
        OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xLegend, uno::Reference<XChartDocument>(m_xChartModel.get()) ) );
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
        OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xTitle, uno::Reference<XChartDocument>(m_xChartModel.get()) ) );
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
