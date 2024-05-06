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

#include <ReferenceSizeProvider.hxx>
#include <RelativeSizeHelper.hxx>
#include <ChartModel.hxx>
#include <DataSeries.hxx>
#include <DataSeriesProperties.hxx>
#include <Diagram.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <Legend.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::chart::DataSeriesProperties;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

ReferenceSizeProvider::ReferenceSizeProvider(
    awt::Size aPageSize,
    const rtl::Reference<::chart::ChartModel> & xChartDoc ) :
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
            xTitleProp->getPropertyValue( u"ReferencePageSize"_ustr) >>= aOldRefSize );

        // set from auto-resize on to off -> adapt font sizes at XFormattedStrings
        if( bHasOldRefSize && ! useAutoScale())
        {
            const uno::Sequence< uno::Reference< XFormattedString > > aStrSeq(
                xTitle->getText());
            for( uno::Reference< XFormattedString > const & formattedStr : aStrSeq )
            {
                RelativeSizeHelper::adaptFontSizes(
                    Reference< beans::XPropertySet >( formattedStr, uno::UNO_QUERY ),
                    aOldRefSize, getPageSize());
            }
        }

        setValuesAtPropertySet( xTitleProp, /* bAdaptFontSizes = */ false );
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void ReferenceSizeProvider::setValuesAtAllDataSeries()
{
    rtl::Reference< Diagram > xDiagram( m_xChartDoc->getFirstChartDiagram());
    if (!xDiagram)
        return;

    // DataSeries/Points
    std::vector< rtl::Reference< DataSeries > > aSeries =
        xDiagram->getDataSeries();

    for (auto const& elem : aSeries)
    {
        // data points
        Sequence< sal_Int32 > aPointIndexes;
        try
        {
            // "AttributedDataPoints"
            if( elem->getFastPropertyValue( PROP_DATASERIES_ATTRIBUTED_DATA_POINTS) >>= aPointIndexes )
            {
                for (sal_Int32 idx : aPointIndexes)
                    setValuesAtPropertySet(
                        elem->getDataPointByIndex( idx ) );
            }
        }
        catch (const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }

        //it is important to correct the datapoint properties first as they do reference the series properties
        setValuesAtPropertySet( elem );
    }
}

void ReferenceSizeProvider::setValuesAtPropertySet(
    const Reference< beans::XPropertySet > & xProp,
    bool bAdaptFontSizes /* = true */ )
{
    if( ! xProp.is())
        return;

    static constexpr OUString aRefSizeName = u"ReferencePageSize"_ustr;

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
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
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
            if( xProp->getPropertyValue( u"ReferencePageSize"_ustr ).hasValue())
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
    const rtl::Reference<::chart::ChartModel> & xChartDoc )
{
    AutoResizeState eResult = AUTO_RESIZE_UNKNOWN;

    // Main Title
    if( xChartDoc.is())
        impl_getAutoResizeFromTitled( xChartDoc, eResult );
    if( eResult == AUTO_RESIZE_AMBIGUOUS )
        return eResult;

    // diagram is needed by the rest of the objects
    rtl::Reference< Diagram > xDiagram = xChartDoc->getFirstChartDiagram();
    if( ! xDiagram.is())
        return eResult;

    // Sub Title
    if( xDiagram.is())
        impl_getAutoResizeFromTitled( xDiagram, eResult );
    if( eResult == AUTO_RESIZE_AMBIGUOUS )
        return eResult;

    // Legend
    rtl::Reference< Legend > xLegend( xDiagram->getLegend2() );
    if( xLegend.is())
        getAutoResizeFromPropSet( xLegend, eResult );
    if( eResult == AUTO_RESIZE_AMBIGUOUS )
        return eResult;

    // Axes (incl. Axis Titles)
    const std::vector< rtl::Reference< Axis > > aAxes = AxisHelper::getAllAxesOfDiagram( xDiagram );
    for( rtl::Reference< Axis > const & axis : aAxes )
    {
        getAutoResizeFromPropSet( axis, eResult );
        impl_getAutoResizeFromTitled( axis, eResult );
        if( eResult == AUTO_RESIZE_AMBIGUOUS )
            return eResult;
    }

    // DataSeries/Points
    std::vector< rtl::Reference< DataSeries > > aSeries =
        xDiagram->getDataSeries();

    for (auto const& elem : aSeries)
    {
        getAutoResizeFromPropSet( elem, eResult );
        if( eResult == AUTO_RESIZE_AMBIGUOUS )
            return eResult;

        // data points
        Sequence< sal_Int32 > aPointIndexes;
        try
        {
            // "AttributedDataPoints"
            if( elem->getFastPropertyValue( PROP_DATASERIES_ATTRIBUTED_DATA_POINTS) >>= aPointIndexes )
            {
                for (sal_Int32 idx : aPointIndexes)
                {
                    getAutoResizeFromPropSet(
                        elem->getDataPointByIndex( idx ), eResult );
                    if( eResult == AUTO_RESIZE_AMBIGUOUS )
                        return eResult;
                }
            }
        }
        catch (const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
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
    impl_setValuesAtTitled( m_xChartDoc );

    // diagram is needed by the rest of the objects
    rtl::Reference< Diagram > xDiagram = m_xChartDoc->getFirstChartDiagram();
    if( ! xDiagram.is())
        return;

    // Sub Title
    impl_setValuesAtTitled( xDiagram );

    // Legend
    rtl::Reference< Legend > xLegend( xDiagram->getLegend2() );
    if( xLegend.is())
        setValuesAtPropertySet( xLegend );

    // Axes (incl. Axis Titles)
    const std::vector< rtl::Reference< Axis > > aAxes = AxisHelper::getAllAxesOfDiagram( xDiagram );
    for( rtl::Reference< Axis > const & axis : aAxes )
    {
        setValuesAtPropertySet( axis );
        impl_setValuesAtTitled( axis );
    }

    // DataSeries/Points
    setValuesAtAllDataSeries();

    // recalculate new state (in case it stays unknown or is ambiguous
    m_bUseAutoScale = (getAutoResizeState( m_xChartDoc ) == AUTO_RESIZE_YES);
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
