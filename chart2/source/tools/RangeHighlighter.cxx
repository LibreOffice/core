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

#include <RangeHighlighter.hxx>
#include <WeakListenerAdapter.hxx>
#include <ChartModel.hxx>
#include <ChartModelHelper.hxx>
#include <DataSourceHelper.hxx>
#include <ObjectIdentifier.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <Diagram.hxx>

#include <com/sun/star/chart2/ScaleData.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/color.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{

const Color defaultPreferredColor = COL_LIGHTBLUE;

void lcl_fillRanges(
    Sequence< chart2::data::HighlightedRange > & rOutRanges,
    const Sequence< OUString >& aRangeStrings,
    Color nPreferredColor,
    sal_Int32 nIndex = -1 )
{
    rOutRanges.realloc( aRangeStrings.getLength());
    auto pOutRanges = rOutRanges.getArray();
    for( sal_Int32 i=0; i<aRangeStrings.getLength(); ++i )
    {
        pOutRanges[i].RangeRepresentation = aRangeStrings[i];
        pOutRanges[i].PreferredColor = sal_Int32(nPreferredColor);
        pOutRanges[i].AllowMerginigWithOtherRanges = false;
        pOutRanges[i].Index = nIndex;
    }
}

} // anonymous namespace

namespace chart
{

RangeHighlighter::RangeHighlighter(
    const rtl::Reference< ChartModel > & xChartModel ) :
        m_xSelectionSupplier(xChartModel->getCurrentController(), uno::UNO_QUERY),
        m_xChartModel( xChartModel ),
        m_nAddedListenerCount( 0 ),
        m_bIncludeHiddenCells(true)
{
}

RangeHighlighter::~RangeHighlighter()
{}

// ____ XRangeHighlighter ____
Sequence< chart2::data::HighlightedRange > SAL_CALL RangeHighlighter::getSelectedRanges()
{
    return m_aSelectedRanges;
}

void RangeHighlighter::determineRanges()
{
    m_aSelectedRanges.realloc( 0 );
    if( !m_xChartModel.is())
        return;
    if( !m_xSelectionSupplier.is())
        return;

    try
    {
        m_bIncludeHiddenCells = ChartModelHelper::isIncludeHiddenCells( m_xChartModel );

        uno::Any aSelection( m_xSelectionSupplier->getSelection());
        const uno::Type& rType = aSelection.getValueType();

        if ( rType == cppu::UnoType<OUString>::get() )
        {
            // @todo??: maybe getSelection() should return a model object rather than a CID

            OUString aCID;
            aSelection >>= aCID;
            if ( !aCID.isEmpty() )
            {
                ObjectType eObjectType = ObjectIdentifier::getObjectType( aCID );
                sal_Int32 nIndex = ObjectIdentifier::getIndexFromParticleOrCID( aCID );
                rtl::Reference< DataSeries > xDataSeries( ObjectIdentifier::getDataSeriesForCID( aCID, m_xChartModel ) );
                if( eObjectType == OBJECTTYPE_LEGEND_ENTRY )
                {
                    OUString aParentParticel( ObjectIdentifier::getFullParentParticle( aCID ) );
                    ObjectType eParentObjectType = ObjectIdentifier::getObjectType( aParentParticel );
                    eObjectType = eParentObjectType;
                    if( eObjectType == OBJECTTYPE_DATA_POINT )
                        nIndex = ObjectIdentifier::getIndexFromParticleOrCID( aParentParticel );
                }

                if( eObjectType == OBJECTTYPE_DATA_POINT || eObjectType == OBJECTTYPE_DATA_LABEL )
                {
                    // Data Point
                    fillRangesForDataPoint( xDataSeries, nIndex );
                    return;
                }
                else if( eObjectType == OBJECTTYPE_DATA_ERRORS_X ||
                         eObjectType == OBJECTTYPE_DATA_ERRORS_Y ||
                         eObjectType == OBJECTTYPE_DATA_ERRORS_Z )
                {
                    // select error bar ranges, or data series, if the style is
                    // not set to FROM_DATA
                    fillRangesForErrorBars( ObjectIdentifier::getObjectPropertySet( aCID, m_xChartModel ), xDataSeries );
                    return;
                }
                else if( xDataSeries.is() )
                {
                    // Data Series
                    fillRangesForDataSeries( xDataSeries );
                    return;
                }
                else if( eObjectType == OBJECTTYPE_AXIS )
                {
                    // Axis (Categories)
                    Reference< chart2::XAxis > xAxis( ObjectIdentifier::getObjectPropertySet( aCID, m_xChartModel ), uno::UNO_QUERY );
                    if( xAxis.is())
                    {
                        fillRangesForCategories( xAxis );
                        return;
                    }
                }
                else if( eObjectType == OBJECTTYPE_PAGE
                         || eObjectType == OBJECTTYPE_DIAGRAM
                         || eObjectType == OBJECTTYPE_DIAGRAM_WALL
                         || eObjectType == OBJECTTYPE_DIAGRAM_FLOOR
                    )
                {
                    // Diagram
                    rtl::Reference< ::chart::Diagram > xDia( ObjectIdentifier::getDiagramForCID( aCID, m_xChartModel ) );
                    if( xDia.is())
                    {
                        fillRangesForDiagram( xDia );
                        return;
                    }
                }
            }
        }
        else if ( rType == cppu::UnoType< drawing::XShape >::get() )
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
            fillRangesForDiagram( m_xChartModel->getFirstChartDiagram() );
            return;
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void RangeHighlighter::fillRangesForDiagram( const rtl::Reference< Diagram > & xDiagram )
{
    Sequence< OUString > aSelectedRanges( DataSourceHelper::getUsedDataRanges( xDiagram ));
    m_aSelectedRanges.realloc( aSelectedRanges.getLength());
    auto pSelectedRanges = m_aSelectedRanges.getArray();
    // @todo: merge ranges
    for( sal_Int32 i=0; i<aSelectedRanges.getLength(); ++i )
    {
        pSelectedRanges[i].RangeRepresentation = aSelectedRanges[i];
        pSelectedRanges[i].Index = -1;
        pSelectedRanges[i].PreferredColor = sal_Int32(defaultPreferredColor);
        pSelectedRanges[i].AllowMerginigWithOtherRanges = true;
    }
}

void RangeHighlighter::fillRangesForDataSeries( const uno::Reference< chart2::XDataSeries > & xSeries )
{
    Reference< chart2::data::XDataSource > xSource( xSeries, uno::UNO_QUERY );
    if( xSource.is())
    {
        lcl_fillRanges( m_aSelectedRanges,
                        ::chart::DataSourceHelper::getRangesFromDataSource( xSource ),
                        defaultPreferredColor );
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
        sal_Int32 nStyle = css::chart::ErrorBarStyle::NONE;
        bUsesRangesAsErrorBars =
            ( xErrorBar.is() &&
              (xErrorBar->getPropertyValue( u"ErrorBarStyle"_ustr) >>= nStyle) &&
              nStyle == css::chart::ErrorBarStyle::FROM_DATA );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    if( bUsesRangesAsErrorBars )
    {
        Reference< chart2::data::XDataSource > xSource( xErrorBar, uno::UNO_QUERY );
        if( xSource.is())
        {
            lcl_fillRanges( m_aSelectedRanges,
                            ::chart::DataSourceHelper::getRangesFromDataSource( xSource ),
                            defaultPreferredColor );
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
                    DataSourceHelper::getRangesFromLabeledDataSequence( aData.Categories ),
                    defaultPreferredColor );
}

void RangeHighlighter::fillRangesForDataPoint( const rtl::Reference< DataSeries > & xDataSeries, sal_Int32 nIndex )
{
    if( !xDataSeries.is())
        return;

    Color nPreferredColor = defaultPreferredColor;
    std::vector< chart2::data::HighlightedRange > aHilightedRanges;
    const std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > & aLSeqSeq( xDataSeries->getDataSequences2());
    for( uno::Reference< chart2::data::XLabeledDataSequence > const & labelDataSeq : aLSeqSeq )
    {
        Reference< chart2::data::XDataSequence > xLabel( labelDataSeq->getLabel());
        Reference< chart2::data::XDataSequence > xValues( labelDataSeq->getValues());

        if( xLabel.is())
            aHilightedRanges.emplace_back(
                    xLabel->getSourceRangeRepresentation(),
                    -1,
                    sal_Int32(nPreferredColor),
                    false );

        sal_Int32 nUnhiddenIndex = DataSeriesHelper::translateIndexFromHiddenToFullSequence( nIndex, xValues, !m_bIncludeHiddenCells );
        if( xValues.is())
            aHilightedRanges.emplace_back(
                    xValues->getSourceRangeRepresentation(),
                    nUnhiddenIndex,
                    sal_Int32(nPreferredColor),
                    false );
    }
    m_aSelectedRanges = comphelper::containerToSequence( aHilightedRanges );
}

void SAL_CALL RangeHighlighter::addSelectionChangeListener( const Reference< view::XSelectionChangeListener >& xListener )
{
    if(!xListener.is())
        return;

    if( m_nAddedListenerCount == 0 )
        startListening();
    std::unique_lock g(m_aMutex);
    maSelectionChangeListeners.addInterface( g, xListener);
    ++m_nAddedListenerCount;

    //bring the new listener up to the current state
    lang::EventObject aEvent( static_cast< lang::XComponent* >( this ) );
    xListener->selectionChanged( aEvent );
}

void SAL_CALL RangeHighlighter::removeSelectionChangeListener( const Reference< view::XSelectionChangeListener >& xListener )
{
    std::unique_lock g(m_aMutex);
    maSelectionChangeListeners.removeInterface( g, xListener );
    --m_nAddedListenerCount;
    if( m_nAddedListenerCount == 0 )
        stopListening();
}

// ____ XSelectionChangeListener ____
void SAL_CALL RangeHighlighter::selectionChanged( const lang::EventObject& /*aEvent*/ )
{
    determineRanges();

    // determine ranges of selected view objects
    // if changed, fire an event
    fireSelectionEvent();
}

void RangeHighlighter::fireSelectionEvent()
{
    std::unique_lock g(m_aMutex);
    if( maSelectionChangeListeners.getLength(g) )
    {
        lang::EventObject aEvent( static_cast< lang::XComponent* >( this ) );
        maSelectionChangeListeners.forEach(g,
            [&aEvent](const css::uno::Reference<view::XSelectionChangeListener>& xListener)
            {
                xListener->selectionChanged(aEvent);
            }
        );
    }
}

void SAL_CALL RangeHighlighter::disposing( const lang::EventObject& Source )
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
void RangeHighlighter::disposing(std::unique_lock<std::mutex>&)
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
