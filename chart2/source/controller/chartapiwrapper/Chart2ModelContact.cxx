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
#include "ChartModelHelper.hxx"
#include "LegendHelper.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "servicenames.hxx"
#include "ObjectIdentifier.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "AxisHelper.hxx"
#include "DiagramHelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{
namespace wrapper
{

Chart2ModelContact::Chart2ModelContact(
    const Reference< uno::XComponentContext > & xContext ) :
        m_xContext( xContext ),
        m_xChartModel( 0 ),
        m_xChartView(0)
{
}

Chart2ModelContact::~Chart2ModelContact()
{
    this->clear();
}

void Chart2ModelContact::setModel( const ::com::sun::star::uno::Reference<
                       ::com::sun::star::frame::XModel >& xChartModel )
{
    this->clear();
    m_xChartModel = xChartModel;
    uno::Reference< lang::XMultiServiceFactory > xTableFactory( xChartModel, uno::UNO_QUERY );
    if( xTableFactory.is() )
    {
        uno::Reference< container::XNameContainer > xDashTable( xTableFactory->createInstance("com.sun.star.drawing.DashTable"), uno::UNO_QUERY );
        uno::Reference< container::XNameContainer > xGradientTable( xTableFactory->createInstance("com.sun.star.drawing.GradientTable"), uno::UNO_QUERY );
        uno::Reference< container::XNameContainer > xHatchTable( xTableFactory->createInstance("com.sun.star.drawing.HatchTable"), uno::UNO_QUERY );
        uno::Reference< container::XNameContainer > xBitmapTable( xTableFactory->createInstance("com.sun.star.drawing.BitmapTable"), uno::UNO_QUERY );
        uno::Reference< container::XNameContainer > xTransparencyGradientTable( xTableFactory->createInstance("com.sun.star.drawing.TransparencyGradientTable"), uno::UNO_QUERY );
        m_aTableMap["LineDashName"] = xDashTable;
        m_aTableMap["FillGradientName"] = xGradientTable;
        m_aTableMap["FillHatchName"] = xHatchTable;
        m_aTableMap["FillBitmapName"] = xBitmapTable;
        m_aTableMap["FillTransparenceGradientName"] = xTransparencyGradientTable;
    }
}

void Chart2ModelContact::clear()
{
    m_xChartModel = uno::WeakReference< frame::XModel >(0);
    m_xChartView.clear();
}

Reference< frame::XModel > Chart2ModelContact::getChartModel() const
{
    return Reference< frame::XModel >( m_xChartModel.get(), uno::UNO_QUERY );
}

Reference< chart2::XChartDocument > Chart2ModelContact::getChart2Document() const
{
    return Reference< chart2::XChartDocument >( m_xChartModel.get(), uno::UNO_QUERY );
}

Reference< chart2::XDiagram > Chart2ModelContact::getChart2Diagram() const
{
    return ChartModelHelper::findDiagram( this->getChartModel() );
}

uno::Reference< lang::XUnoTunnel > Chart2ModelContact::getChartView() const
{
    if(!m_xChartView.is())
    {
        // get the chart view
        Reference<frame::XModel> xModel(m_xChartModel);
        uno::Reference< lang::XMultiServiceFactory > xFact( xModel, uno::UNO_QUERY );
        if( xFact.is() )
            m_xChartView = Reference< lang::XUnoTunnel >( xFact->createInstance( CHART_VIEW_SERVICE_NAME ), uno::UNO_QUERY );
    }
    return m_xChartView;
}

ExplicitValueProvider* Chart2ModelContact::getExplicitValueProvider() const
{
    getChartView();
    if(!m_xChartView.is())
        return 0;

    //obtain the ExplicitValueProvider from the chart view
    ExplicitValueProvider* pProvider = reinterpret_cast<ExplicitValueProvider*>(m_xChartView->getSomething(
        ExplicitValueProvider::getUnoTunnelId() ));
    return pProvider;
}

uno::Reference< drawing::XDrawPage > Chart2ModelContact::getDrawPage()
{
    uno::Reference< drawing::XDrawPage > xResult;
    ExplicitValueProvider* pProvider( getExplicitValueProvider() );
    if( pProvider )
    {
        xResult.set( pProvider->getDrawModelWrapper()->getMainDrawPage() );
    }
    return xResult;
}

sal_Bool Chart2ModelContact::getExplicitValuesForAxis(
    const Reference< XAxis > & xAxis,
    ExplicitScaleData &  rOutExplicitScale,
    ExplicitIncrementData & rOutExplicitIncrement )
{
    ExplicitValueProvider* pProvider( getExplicitValueProvider() );
    if( pProvider )
    {
        return pProvider->getExplicitValuesForAxis(
            xAxis, rOutExplicitScale, rOutExplicitIncrement );
    }
    return sal_False;
}

sal_Int32 Chart2ModelContact::getExplicitNumberFormatKeyForAxis(
            const Reference< chart2::XAxis >& xAxis )
{
    Reference< chart2::XCoordinateSystem > xCooSys(
        AxisHelper::getCoordinateSystemOfAxis(
              xAxis, ChartModelHelper::findDiagram( m_xChartModel ) ) );

    return ExplicitValueProvider::getExplicitNumberFormatKeyForAxis( xAxis, xCooSys
              , Reference< util::XNumberFormatsSupplier >( m_xChartModel.get(), uno::UNO_QUERY ) );
}

sal_Int32 Chart2ModelContact::getExplicitNumberFormatKeyForSeries(
            const Reference< chart2::XDataSeries >& xSeries )
{
    return ExplicitValueProvider::getExplicitNumberFormatKeyForDataLabel(
        uno::Reference< beans::XPropertySet >( xSeries, uno::UNO_QUERY ),
        xSeries,
        -1 /*-1 for whole series*/,
        ChartModelHelper::findDiagram( m_xChartModel )
        );
}

awt::Size Chart2ModelContact::GetPageSize() const
{
    return ChartModelHelper::getPageSize(m_xChartModel);
}

awt::Rectangle Chart2ModelContact::SubstractAxisTitleSizes( const awt::Rectangle& rPositionRect )
{
    awt::Rectangle aRect = ExplicitValueProvider::substractAxisTitleSizes(
        m_xChartModel, getChartView(), rPositionRect );
    return aRect;
}

awt::Rectangle Chart2ModelContact::GetDiagramRectangleIncludingTitle() const
{
    awt::Rectangle aRect( GetDiagramRectangleIncludingAxes() );

    //add axis title sizes to the diagram size
    aRect = ExplicitValueProvider::addAxisTitleSizes(
        m_xChartModel, getChartView(), aRect );

    return aRect;
}

awt::Rectangle Chart2ModelContact::GetDiagramRectangleIncludingAxes() const
{
    awt::Rectangle aRect(0,0,0,0);
    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( m_xChartModel ) );

    if( DiagramPositioningMode_INCLUDING == DiagramHelper::getDiagramPositioningMode( xDiagram ) )
        aRect = DiagramHelper::getDiagramRectangleFromModel(m_xChartModel);
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
    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( m_xChartModel ) );

    if( DiagramPositioningMode_EXCLUDING == DiagramHelper::getDiagramPositioningMode( xDiagram ) )
        aRect = DiagramHelper::getDiagramRectangleFromModel(m_xChartModel);
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
        uno::Reference< chart2::XLegend > xLegend( LegendHelper::getLegend( m_xChartModel ) );
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
        uno::Reference< chart2::XLegend > xLegend( LegendHelper::getLegend( m_xChartModel ) );
        OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xLegend, m_xChartModel ) );
        aPoint = ToPoint( pProvider->getRectangleOfObject( aCID ) );
    }
    return aPoint;
}

awt::Size Chart2ModelContact::GetTitleSize( const uno::Reference<
        ::com::sun::star::chart2::XTitle > & xTitle ) const
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

awt::Point Chart2ModelContact::GetTitlePosition( const uno::Reference<
        ::com::sun::star::chart2::XTitle > & xTitle ) const
{
    awt::Point aPoint;
    ExplicitValueProvider* pProvider( getExplicitValueProvider() );
    if( pProvider && xTitle.is() )
    {
        OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xTitle, m_xChartModel ) );
        aPoint = ToPoint( pProvider->getRectangleOfObject( aCID ) );
    }
    return aPoint;
}

awt::Size Chart2ModelContact::GetAxisSize( const uno::Reference<
        ::com::sun::star::chart2::XAxis > & xAxis ) const
{
    awt::Size aSize;
    ExplicitValueProvider* pProvider( getExplicitValueProvider() );
    if( pProvider && xAxis.is() )
    {
        OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xAxis, m_xChartModel ) );
        aSize = ToSize( pProvider->getRectangleOfObject( aCID ) );
    }
    return aSize;
}

awt::Point Chart2ModelContact::GetAxisPosition( const uno::Reference<
        ::com::sun::star::chart2::XAxis > & xAxis ) const
{
    awt::Point aPoint;
    ExplicitValueProvider* pProvider( getExplicitValueProvider() );
    if( pProvider && xAxis.is() )
    {
        OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xAxis, m_xChartModel ) );
        aPoint = ToPoint( pProvider->getRectangleOfObject( aCID ) );
    }
    return aPoint;
}

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
