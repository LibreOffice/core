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

#include "RangeHighlighter.hxx"
#include "WeakListenerAdapter.hxx"
#include "ChartModelHelper.hxx"
#include "DataSourceHelper.hxx"
#include "ContainerHelper.hxx"
#include "macros.hxx"
#include "ObjectIdentifier.hxx"
#include "DataSeriesHelper.hxx"

#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/drawing/XShape.hpp>

#define PREFERED_DEFAULT_COLOR 0x0000ff

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{

void lcl_fillRanges(
    Sequence< chart2::data::HighlightedRange > & rOutRanges,
    Sequence< OUString > aRangeStrings,
    sal_Int32 nPreferredColor = PREFERED_DEFAULT_COLOR,
    sal_Int32 nIndex = -1 )
{
    rOutRanges.realloc( aRangeStrings.getLength());
    for( sal_Int32 i=0; i<aRangeStrings.getLength(); ++i )
    {
        rOutRanges[i].RangeRepresentation = aRangeStrings[i];
        rOutRanges[i].PreferredColor = nPreferredColor;
        rOutRanges[i].AllowMerginigWithOtherRanges = sal_False;
        rOutRanges[i].Index = nIndex;
    }
}

} // anonymous namespace

namespace chart
{

RangeHighlighter::RangeHighlighter(
    const Reference< view::XSelectionSupplier > & xSelectionSupplier ) :
        impl::RangeHighlighter_Base( m_aMutex ),
        m_xSelectionSupplier( xSelectionSupplier ),
        m_nAddedListenerCount( 0 ),
        m_bIncludeHiddenCells(true)
{
}

RangeHighlighter::~RangeHighlighter()
{}

// ____ XRangeHighlighter ____
Sequence< chart2::data::HighlightedRange > SAL_CALL RangeHighlighter::getSelectedRanges()
    throw (uno::RuntimeException)
{
    return m_aSelectedRanges;
}

void RangeHighlighter::determineRanges()
{
    m_aSelectedRanges.realloc( 0 );
    if( m_xSelectionSupplier.is())
    {
        try
        {
            Reference< frame::XController > xController( m_xSelectionSupplier, uno::UNO_QUERY );
            Reference< frame::XModel > xChartModel;
            if( xController.is())
                xChartModel.set( xController->getModel());

            m_bIncludeHiddenCells = ChartModelHelper::isIncludeHiddenCells( xChartModel );

            uno::Any aSelection( m_xSelectionSupplier->getSelection());
            const uno::Type& rType = aSelection.getValueType();

            if ( rType == ::getCppuType( static_cast< const OUString* >( 0 ) ) )
            {
                // @todo??: maybe getSelection() should return a model object rather than a CID

                OUString aCID;
                aSelection >>= aCID;
                if ( !aCID.isEmpty() )
                {
                    ObjectType eObjectType = ObjectIdentifier::getObjectType( aCID );
                    sal_Int32 nIndex = ObjectIdentifier::getIndexFromParticleOrCID( aCID );
                    Reference< chart2::XDataSeries > xDataSeries( ObjectIdentifier::getDataSeriesForCID( aCID, xChartModel ) );
                    if( OBJECTTYPE_LEGEND_ENTRY == eObjectType )
                    {
                        OUString aParentParticel( ObjectIdentifier::getFullParentParticle( aCID ) );
                        ObjectType eParentObjectType = ObjectIdentifier::getObjectType( aParentParticel );
                        eObjectType = eParentObjectType;
                        if( OBJECTTYPE_DATA_POINT == eObjectType )
                            nIndex = ObjectIdentifier::getIndexFromParticleOrCID( aParentParticel );
                    }

                    if( OBJECTTYPE_DATA_POINT == eObjectType || OBJECTTYPE_DATA_LABEL == eObjectType )
                    {
                        // Data Point
                        fillRangesForDataPoint( xDataSeries, nIndex );
                        return;
                    }
                    else if( OBJECTTYPE_DATA_ERRORS_X == eObjectType ||
                             OBJECTTYPE_DATA_ERRORS_Y == eObjectType ||
                             OBJECTTYPE_DATA_ERRORS_Z == eObjectType )
                    {
                        // select error bar ranges, or data series, if the style is
                        // not set to FROM_DATA
                        fillRangesForErrorBars( ObjectIdentifier::getObjectPropertySet( aCID, xChartModel ), xDataSeries );
                        return;
                    }
                    else if( xDataSeries.is() )
                    {
                        // Data Series
                        fillRangesForDataSeries( xDataSeries );
                        return;
                    }
                    else if( OBJECTTYPE_AXIS == eObjectType )
                    {
                        // Axis (Categories)
                        Reference< chart2::XAxis > xAxis( ObjectIdentifier::getObjectPropertySet( aCID, xChartModel ), uno::UNO_QUERY );
                        if( xAxis.is())
                        {
                            fillRangesForCategories( xAxis );
                            return;
                        }
                    }
                    else if( OBJECTTYPE_PAGE == eObjectType
                             || OBJECTTYPE_DIAGRAM == eObjectType
                             || OBJECTTYPE_DIAGRAM_WALL == eObjectType
                             || OBJECTTYPE_DIAGRAM_FLOOR == eObjectType
                        )
                    {
                        // Diagram
                        Reference< chart2::XDiagram > xDia( ObjectIdentifier::getDiagramForCID( aCID, xChartModel ) );
                        if( xDia.is())
                        {
                            fillRangesForDiagram( xDia );
                            return;
                        }
                    }
                }
            }
            else if ( rType == ::getCppuType( static_cast< const Reference< drawing::XShape >* >( 0 ) ) )
            {
                // #i12587# support for shapes in chart
                Reference< drawing::XShape > xShape;
                aSelection >>= xShape;
                if ( xShape.is() )
                {
                    return;
                }
            }
            else
            {
                //if nothing is selected select all ranges
                Reference< chart2::XChartDocument > xChartDoc( xChartModel, uno::UNO_QUERY_THROW );
                fillRangesForDiagram( xChartDoc->getFirstDiagram() );
                return;
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

void RangeHighlighter::fillRangesForDiagram( const Reference< chart2::XDiagram > & xDiagram )
{
    Sequence< OUString > aSelectedRanges( DataSourceHelper::getUsedDataRanges( xDiagram ));
    m_aSelectedRanges.realloc( aSelectedRanges.getLength());
    // @todo: merge ranges
    for( sal_Int32 i=0; i<aSelectedRanges.getLength(); ++i )
    {
        m_aSelectedRanges[i].RangeRepresentation = aSelectedRanges[i];
        m_aSelectedRanges[i].Index = -1;
        m_aSelectedRanges[i].PreferredColor = PREFERED_DEFAULT_COLOR;
        m_aSelectedRanges[i].AllowMerginigWithOtherRanges = sal_True;
    }
}

void RangeHighlighter::fillRangesForDataSeries( const uno::Reference< chart2::XDataSeries > & xSeries )
{
    Reference< chart2::data::XDataSource > xSource( xSeries, uno::UNO_QUERY );
    if( xSource.is())
    {
        sal_Int32 nPreferredColor = PREFERED_DEFAULT_COLOR;
        lcl_fillRanges( m_aSelectedRanges,
                        ::chart::DataSourceHelper::getRangesFromDataSource( xSource ),
                        nPreferredColor );
    }
}

void RangeHighlighter::fillRangesForErrorBars(
    const uno::Reference< beans::XPropertySet > & xErrorBar,
    const uno::Reference< chart2::XDataSeries > & xSeries )
{
    // only show error bar ranges, if the style is set to FROM_DATA
    bool bUsesRangesAsErrorBars = false;
    try
    {
        sal_Int32 nStyle = ::com::sun::star::chart::ErrorBarStyle::NONE;
        bUsesRangesAsErrorBars =
            ( xErrorBar.is() &&
              (xErrorBar->getPropertyValue( "ErrorBarStyle") >>= nStyle) &&
              nStyle == ::com::sun::star::chart::ErrorBarStyle::FROM_DATA );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    if( bUsesRangesAsErrorBars )
    {
        Reference< chart2::data::XDataSource > xSource( xErrorBar, uno::UNO_QUERY );
        if( xSource.is())
        {
            sal_Int32 nPreferredColor = PREFERED_DEFAULT_COLOR;
            lcl_fillRanges( m_aSelectedRanges,
                            ::chart::DataSourceHelper::getRangesFromDataSource( xSource ),
                            nPreferredColor );
        }
    }
    else
    {
        fillRangesForDataSeries( xSeries );
    }
}

void RangeHighlighter::fillRangesForCategories( const Reference< chart2::XAxis > & xAxis )
{
    if( ! xAxis.is())
        return;
    chart2::ScaleData aData( xAxis->getScaleData());
    lcl_fillRanges( m_aSelectedRanges,
                    DataSourceHelper::getRangesFromLabeledDataSequence( aData.Categories ));
}

void RangeHighlighter::fillRangesForDataPoint( const Reference< uno::XInterface > & xDataSeries, sal_Int32 nIndex )
{
    if( xDataSeries.is())
    {
        Reference< chart2::data::XDataSource > xSource( xDataSeries, uno::UNO_QUERY );
        if( xSource.is() )
        {
            sal_Int32 nPreferredColor = PREFERED_DEFAULT_COLOR;
            ::std::vector< chart2::data::HighlightedRange > aHilightedRanges;
            Sequence< Reference< chart2::data::XLabeledDataSequence > > aLSeqSeq( xSource->getDataSequences());
            for( sal_Int32 i=0; i<aLSeqSeq.getLength(); ++i )
            {
                Reference< chart2::data::XDataSequence > xLabel( aLSeqSeq[i]->getLabel());
                Reference< chart2::data::XDataSequence > xValues( aLSeqSeq[i]->getValues());

                if( xLabel.is())
                    aHilightedRanges.push_back(
                        chart2::data::HighlightedRange(
                            xLabel->getSourceRangeRepresentation(),
                            -1,
                            nPreferredColor,
                            sal_False ));

                sal_Int32 nUnhiddenIndex = DataSeriesHelper::translateIndexFromHiddenToFullSequence( nIndex, xValues, !m_bIncludeHiddenCells );
                if( xValues.is())
                    aHilightedRanges.push_back(
                        chart2::data::HighlightedRange(
                            xValues->getSourceRangeRepresentation(),
                            nUnhiddenIndex,
                            nPreferredColor,
                            sal_False ));
            }
            m_aSelectedRanges = ContainerHelper::ContainerToSequence( aHilightedRanges );
        }
    }
}

void SAL_CALL RangeHighlighter::addSelectionChangeListener( const Reference< view::XSelectionChangeListener >& xListener )
    throw (uno::RuntimeException)
{
    if(!xListener.is())
        return;

    if( m_nAddedListenerCount == 0 )
        startListening();
    rBHelper.addListener( ::getCppuType( & xListener ), xListener);
    ++m_nAddedListenerCount;

    //bring the new listener up to the current state
    lang::EventObject aEvent( static_cast< lang::XComponent* >( this ) );
    xListener->selectionChanged( aEvent );
}

void SAL_CALL RangeHighlighter::removeSelectionChangeListener( const Reference< view::XSelectionChangeListener >& xListener )
    throw (uno::RuntimeException)
{
    rBHelper.removeListener( ::getCppuType( & xListener ), xListener );
    --m_nAddedListenerCount;
    if( m_nAddedListenerCount == 0 )
        stopListening();
}

// ____ XSelectionChangeListener ____
void SAL_CALL RangeHighlighter::selectionChanged( const lang::EventObject& /*aEvent*/ )
    throw (uno::RuntimeException)
{
    determineRanges();

    // determine ranges of selected view objects
    // if changed, fire an event
    fireSelectionEvent();
}

void RangeHighlighter::fireSelectionEvent()
{
    ::cppu::OInterfaceContainerHelper* pIC = rBHelper.getContainer(
        ::getCppuType((const uno::Reference< view::XSelectionChangeListener >*)0) );
    if( pIC )
    {
        lang::EventObject aEvent( static_cast< lang::XComponent* >( this ) );
        ::cppu::OInterfaceIteratorHelper aIt( *pIC );
        while( aIt.hasMoreElements() )
        {
            uno::Reference< view::XSelectionChangeListener > xListener( aIt.next(), uno::UNO_QUERY );
            if( xListener.is() )
                xListener->selectionChanged( aEvent );
        }
    }
}

void SAL_CALL RangeHighlighter::disposing( const lang::EventObject& Source )
    throw (uno::RuntimeException)
{
    if( Source.Source == m_xSelectionSupplier )
    {
        m_xSelectionSupplier.clear();
        m_aSelectedRanges.realloc( 0 );
        fireSelectionEvent();
    }
}

void RangeHighlighter::startListening()
{
    if( m_xSelectionSupplier.is())
    {
        if( ! m_xListener.is())
        {
            m_xListener.set( new WeakSelectionChangeListenerAdapter( this ));
            determineRanges();
        }
        m_xSelectionSupplier->addSelectionChangeListener( m_xListener );
    }
}

void RangeHighlighter::stopListening()
{
    if( m_xSelectionSupplier.is() && m_xListener.is())
    {
        m_xSelectionSupplier->removeSelectionChangeListener( m_xListener );
        m_xListener.clear();
    }
}

// ____ WeakComponentImplHelperBase ____
// is called when dispose() is called at this component
void SAL_CALL RangeHighlighter::disposing()
{
    // @todo: remove listener. Currently the controller shows an assertion
    // because it is already disposed
//     stopListening();
    m_xListener.clear();
    m_xSelectionSupplier.clear();
    m_nAddedListenerCount =  0;
    m_aSelectedRanges.realloc( 0 );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
