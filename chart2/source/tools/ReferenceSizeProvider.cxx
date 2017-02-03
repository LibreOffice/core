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

#include "ReferenceSizeProvider.hxx"
#include "RelativeSizeHelper.hxx"
#include "ChartModelHelper.hxx"
#include "DiagramHelper.hxx"
#include "macros.hxx"
#include "AxisHelper.hxx"
#include "DataSeriesHelper.hxx"

#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>

#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

ReferenceSizeProvider::ReferenceSizeProvider(
    awt::Size aPageSize,
    const Reference< XChartDocument > & xChartDoc ) :
        m_aPageSize( aPageSize ),
        m_xChartDoc( xChartDoc ),
        m_bUseAutoScale( getAutoResizeState( xChartDoc ) == AUTO_RESIZE_YES )
{}

void ReferenceSizeProvider::impl_setValuesAtTitled(
    const Reference< XTitled > & xTitled )
{
    if( xTitled.is())
    {
        Reference< XTitle > xTitle( xTitled->getTitleObject());
        if( xTitle.is())
            setValuesAtTitle( xTitle );
    }
}

void ReferenceSizeProvider::setValuesAtTitle(
    const Reference< XTitle > & xTitle )
{
    try
    {
        Reference< beans::XPropertySet > xTitleProp( xTitle, uno::UNO_QUERY_THROW );
        awt::Size aOldRefSize;
        bool bHasOldRefSize(
            xTitleProp->getPropertyValue( "ReferencePageSize") >>= aOldRefSize );

        // set from auto-resize on to off -> adapt font sizes at XFormattedStrings
        if( bHasOldRefSize && ! useAutoScale())
        {
            uno::Sequence< uno::Reference< XFormattedString > > aStrSeq(
                xTitle->getText());
            for( sal_Int32 i=0; i<aStrSeq.getLength(); ++i )
            {
                RelativeSizeHelper::adaptFontSizes(
                    Reference< beans::XPropertySet >( aStrSeq[i], uno::UNO_QUERY ),
                    aOldRefSize, getPageSize());
            }
        }

        setValuesAtPropertySet( xTitleProp, /* bAdaptFontSizes = */ false );
    }
    catch (const uno::Exception& ex)
    {
        ASSERT_EXCEPTION( ex );
    }
}

void ReferenceSizeProvider::setValuesAtAllDataSeries()
{
    Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( m_xChartDoc ));

    // DataSeries/Points
    ::std::vector< Reference< XDataSeries > > aSeries(
        DiagramHelper::getDataSeriesFromDiagram( xDiagram ));

    for( ::std::vector< Reference< XDataSeries > >::const_iterator aIt( aSeries.begin());
         aIt != aSeries.end(); ++aIt )
    {
        Reference< beans::XPropertySet > xSeriesProp( *aIt, uno::UNO_QUERY );
        if( xSeriesProp.is())
        {
            // data points
            Sequence< sal_Int32 > aPointIndexes;
            try
            {
                if( xSeriesProp->getPropertyValue( "AttributedDataPoints") >>= aPointIndexes )
                {
                    for( sal_Int32 i=0; i< aPointIndexes.getLength(); ++i )
                        setValuesAtPropertySet(
                            (*aIt)->getDataPointByIndex( aPointIndexes[i] ) );
                }
            }
            catch (const uno::Exception& ex)
            {
                ASSERT_EXCEPTION( ex );
            }

            //it is important to correct the datapoint properties first as they do reference the series properties
            setValuesAtPropertySet( xSeriesProp );
        }
    }
}

void ReferenceSizeProvider::setValuesAtPropertySet(
    const Reference< beans::XPropertySet > & xProp,
    bool bAdaptFontSizes /* = true */ )
{
    if( ! xProp.is())
        return;

    static const char aRefSizeName[] = "ReferencePageSize";

    try
    {
        awt::Size aRefSize( getPageSize() );
        awt::Size aOldRefSize;
        bool bHasOldRefSize( xProp->getPropertyValue( aRefSizeName ) >>= aOldRefSize );

        if( useAutoScale())
        {
            if( ! bHasOldRefSize )
                xProp->setPropertyValue( aRefSizeName, uno::Any( aRefSize ));
        }
        else
        {
            if( bHasOldRefSize )
            {
                xProp->setPropertyValue( aRefSizeName, uno::Any());

                // adapt font sizes
                if( bAdaptFontSizes )
                    RelativeSizeHelper::adaptFontSizes( xProp, aOldRefSize, aRefSize );
            }
        }
    }
    catch (const uno::Exception& ex)
    {
        ASSERT_EXCEPTION( ex );
    }
}

void ReferenceSizeProvider::getAutoResizeFromPropSet(
    const Reference< beans::XPropertySet > & xProp,
    ReferenceSizeProvider::AutoResizeState & rInOutState )
{
    AutoResizeState eSingleState = AUTO_RESIZE_UNKNOWN;

    if( xProp.is())
    {
        try
        {
            if( xProp->getPropertyValue( "ReferencePageSize" ).hasValue())
                eSingleState = AUTO_RESIZE_YES;
            else
                eSingleState = AUTO_RESIZE_NO;
        }
        catch (const uno::Exception&)
        {
            // unknown property -> state stays unknown
        }
    }

    // current state unknown => nothing changes.  Otherwise if current state
    // differs from state so far, we have an ambiguity
    if( rInOutState == AUTO_RESIZE_UNKNOWN )
    {
        rInOutState = eSingleState;
    }
    else if( eSingleState != AUTO_RESIZE_UNKNOWN &&
        eSingleState != rInOutState )
    {
        rInOutState = AUTO_RESIZE_AMBIGUOUS;
    }
}

void ReferenceSizeProvider::impl_getAutoResizeFromTitled(
    const Reference< XTitled > & xTitled,
    ReferenceSizeProvider::AutoResizeState & rInOutState )
{
    if( xTitled.is())
    {
        Reference< beans::XPropertySet > xProp( xTitled->getTitleObject(), uno::UNO_QUERY );
        if( xProp.is())
            getAutoResizeFromPropSet( xProp, rInOutState );
    }
}

/** Retrieves the state auto-resize from all objects that support this
    feature.  If all objects return the same state, AUTO_RESIZE_YES or
    AUTO_RESIZE_NO is returned.

    If no object supporting the feature is found, AUTO_RESIZE_UNKNOWN is
    returned.  If there are multiple objects, some with state YES and some
    with state NO, AUTO_RESIZE_AMBIGUOUS is returned.
*/
ReferenceSizeProvider::AutoResizeState ReferenceSizeProvider::getAutoResizeState(
    const Reference< XChartDocument > & xChartDoc )
{
    AutoResizeState eResult = AUTO_RESIZE_UNKNOWN;

    // Main Title
    Reference< XTitled > xDocTitled( xChartDoc, uno::UNO_QUERY );
    if( xDocTitled.is())
        impl_getAutoResizeFromTitled( xDocTitled, eResult );
    if( eResult == AUTO_RESIZE_AMBIGUOUS )
        return eResult;

    // diagram is needed by the rest of the objects
    Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartDoc ), uno::UNO_QUERY );
    if( ! xDiagram.is())
        return eResult;

    // Sub Title
    Reference< XTitled > xDiaTitled( xDiagram, uno::UNO_QUERY );
    if( xDiaTitled.is())
        impl_getAutoResizeFromTitled( xDiaTitled, eResult );
    if( eResult == AUTO_RESIZE_AMBIGUOUS )
        return eResult;

    // Legend
    Reference< beans::XPropertySet > xLegendProp( xDiagram->getLegend(), uno::UNO_QUERY );
    if( xLegendProp.is())
        getAutoResizeFromPropSet( xLegendProp, eResult );
    if( eResult == AUTO_RESIZE_AMBIGUOUS )
        return eResult;

    // Axes (incl. Axis Titles)
    Sequence< Reference< XAxis > > aAxes( AxisHelper::getAllAxesOfDiagram( xDiagram ) );
    for( sal_Int32 i=0; i<aAxes.getLength(); ++i )
    {
        Reference< beans::XPropertySet > xProp( aAxes[i], uno::UNO_QUERY );
        if( xProp.is())
            getAutoResizeFromPropSet( xProp, eResult );
        Reference< XTitled > xTitled( aAxes[i], uno::UNO_QUERY );
        if( xTitled.is())
        {
            impl_getAutoResizeFromTitled( xTitled, eResult );
            if( eResult == AUTO_RESIZE_AMBIGUOUS )
                return eResult;
        }
    }

    // DataSeries/Points
    ::std::vector< Reference< XDataSeries > > aSeries(
        DiagramHelper::getDataSeriesFromDiagram( xDiagram ));

    for( ::std::vector< Reference< XDataSeries > >::const_iterator aIt( aSeries.begin());
         aIt != aSeries.end(); ++aIt )
    {
        Reference< beans::XPropertySet > xSeriesProp( *aIt, uno::UNO_QUERY );
        if( xSeriesProp.is())
        {
            getAutoResizeFromPropSet( xSeriesProp, eResult );
            if( eResult == AUTO_RESIZE_AMBIGUOUS )
                return eResult;

            // data points
            Sequence< sal_Int32 > aPointIndexes;
            try
            {
                if( xSeriesProp->getPropertyValue( "AttributedDataPoints") >>= aPointIndexes )
                {
                    for( sal_Int32 i=0; i< aPointIndexes.getLength(); ++i )
                    {
                        getAutoResizeFromPropSet(
                            (*aIt)->getDataPointByIndex( aPointIndexes[i] ), eResult );
                        if( eResult == AUTO_RESIZE_AMBIGUOUS )
                            return eResult;
                    }
                }
            }
            catch (const uno::Exception& ex)
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }

    return eResult;
}

void ReferenceSizeProvider::toggleAutoResizeState()
{
    setAutoResizeState( m_bUseAutoScale ? AUTO_RESIZE_NO : AUTO_RESIZE_YES );
}

/** sets the auto-resize at all objects that support this feature for text.
    eNewState must be either AUTO_RESIZE_YES or AUTO_RESIZE_NO
*/
void ReferenceSizeProvider::setAutoResizeState( ReferenceSizeProvider::AutoResizeState eNewState )
{
    m_bUseAutoScale = (eNewState == AUTO_RESIZE_YES);

    // Main Title
    impl_setValuesAtTitled( Reference< XTitled >( m_xChartDoc, uno::UNO_QUERY ));

    // diagram is needed by the rest of the objects
    Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( m_xChartDoc ), uno::UNO_QUERY );
    if( ! xDiagram.is())
        return;

    // Sub Title
    impl_setValuesAtTitled( Reference< XTitled >( xDiagram, uno::UNO_QUERY ));

    // Legend
    Reference< beans::XPropertySet > xLegendProp( xDiagram->getLegend(), uno::UNO_QUERY );
    if( xLegendProp.is())
        setValuesAtPropertySet( xLegendProp );

    // Axes (incl. Axis Titles)
    Sequence< Reference< XAxis > > aAxes( AxisHelper::getAllAxesOfDiagram( xDiagram ) );
    for( sal_Int32 i=0; i<aAxes.getLength(); ++i )
    {
        Reference< beans::XPropertySet > xProp( aAxes[i], uno::UNO_QUERY );
        if( xProp.is())
            setValuesAtPropertySet( xProp );
        impl_setValuesAtTitled( Reference< XTitled >( aAxes[i], uno::UNO_QUERY ));
    }

    // DataSeries/Points
    setValuesAtAllDataSeries();

    // recalculate new state (in case it stays unknown or is ambiguous
    m_bUseAutoScale = (getAutoResizeState( m_xChartDoc ) == AUTO_RESIZE_YES);
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
