/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

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
using ::rtl::OUString;

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
        uno::Reference< container::XNameContainer > xDashTable( xTableFactory->createInstance( C2U( "com.sun.star.drawing.DashTable" ) ), uno::UNO_QUERY );
        uno::Reference< container::XNameContainer > xGradientTable( xTableFactory->createInstance( C2U( "com.sun.star.drawing.GradientTable" ) ), uno::UNO_QUERY );
        uno::Reference< container::XNameContainer > xHatchTable( xTableFactory->createInstance( C2U( "com.sun.star.drawing.HatchTable" ) ), uno::UNO_QUERY );
        uno::Reference< container::XNameContainer > xBitmapTable( xTableFactory->createInstance( C2U( "com.sun.star.drawing.BitmapTable" ) ), uno::UNO_QUERY );
        uno::Reference< container::XNameContainer > xTransparencyGradientTable( xTableFactory->createInstance( C2U( "com.sun.star.drawing.TransparencyGradientTable" ) ), uno::UNO_QUERY );
        //C2U( "com.sun.star.drawing.MarkerTable" )
        m_aTableMap[ C2U( "LineDashName" ) ] = xDashTable;
        m_aTableMap[ C2U( "FillGradientName" ) ] = xGradientTable;
        m_aTableMap[ C2U( "FillHatchName" ) ] = xHatchTable;
        m_aTableMap[ C2U( "FillBitmapName" ) ] = xBitmapTable;
        m_aTableMap[ C2U( "FillTransparenceGradientName" ) ] = xTransparencyGradientTable;
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

//-----------------------------------------------------------------------------

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
            aRect = pProvider->getRectangleOfObject( C2U("PlotAreaIncludingAxes") );
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
        rtl::OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xLegend, m_xChartModel ) );
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
        rtl::OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xLegend, m_xChartModel ) );
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
        rtl::OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xTitle, m_xChartModel ) );
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
        rtl::OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xTitle, m_xChartModel ) );
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
        rtl::OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xAxis, m_xChartModel ) );
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
        rtl::OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xAxis, m_xChartModel ) );
        aPoint = ToPoint( pProvider->getRectangleOfObject( aCID ) );
    }
    return aPoint;
}

} //  namespace wrapper
} //  namespace chart
