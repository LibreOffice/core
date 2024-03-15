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

#include <StatisticsHelper.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <ErrorBar.hxx>
#include <unonames.hxx>

#include <rtl/ustrbuf.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/chart2/data/LabeledDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <comphelper/diagnose_ex.hxx>

#include <cmath>
#include <limits>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using namespace ::com::sun::star;

namespace
{

double lcl_getVariance( const Sequence< double > & rData, sal_Int32 & rOutValidCount )
{
    const sal_Int32 nCount = rData.getLength();
    rOutValidCount = nCount;

    double fSum = 0.0;
    double fQuadSum = 0.0;

    for( sal_Int32 i = 0; i < nCount; ++i )
    {
        const double fData = rData[i];
        if( std::isnan( fData ))
            --rOutValidCount;
        else
        {
            fSum     += fData;
            fQuadSum += fData * fData;
        }
    }

    if( rOutValidCount == 0 )
        return std::numeric_limits<double>::quiet_NaN();

    const double fN = static_cast< double >( rOutValidCount );
    return (fQuadSum - fSum*fSum/fN) / fN;
}

uno::Reference< chart2::data::XLabeledDataSequence > lcl_getErrorBarLabeledSequence(
    const Reference< chart2::data::XDataSource > & xDataSource,
    bool bPositiveValue, bool bYError,
    OUString & rOutRoleNameUsed )
{
    OUStringBuffer aRole( "error-bars-");
    if( bYError )
        aRole.append( 'y');
    else
        aRole.append( 'x');

    OUString aPlainRole = aRole.makeStringAndClear();
    aRole.append( aPlainRole + "-" );

    if( bPositiveValue )
        aRole.append( "positive" );
    else
        aRole.append( "negative" );

    OUString aLongRole = aRole.makeStringAndClear();
    uno::Reference< chart2::data::XLabeledDataSequence > xLSeq =
        ::chart::DataSeriesHelper::getDataSequenceByRole( xDataSource, aLongRole );
    // try role without "-negative" or "-positive" postfix
    if( xLSeq.is())
        rOutRoleNameUsed = aLongRole;
    else
    {
        xLSeq = ::chart::DataSeriesHelper::getDataSequenceByRole( xDataSource, aPlainRole );
        if( xLSeq.is())
            rOutRoleNameUsed = aPlainRole;
        else
            rOutRoleNameUsed = aLongRole;
    }

    return xLSeq;
}

void lcl_setRole(
    const Reference< chart2::data::XDataSequence > & xNewSequence,
    const OUString & rRole )
{
    Reference< beans::XPropertySet > xSeqProp( xNewSequence, uno::UNO_QUERY );
    if( xSeqProp.is())
        xSeqProp->setPropertyValue( "Role", uno::Any( rRole ));
}

void lcl_addSequenceToDataSource(
    const Reference< chart2::data::XDataSource > & xDataSource,
    const Reference< chart2::data::XDataSequence > & xNewSequence,
    const OUString & rRole )
{
    Reference< chart2::data::XDataSink > xSink( xDataSource, uno::UNO_QUERY );
    Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    if( ! xSink.is() )
        return;

    Reference< chart2::data::XLabeledDataSequence > xLSeq( chart2::data::LabeledDataSequence::create(xContext), uno::UNO_QUERY_THROW );

    lcl_setRole( xNewSequence, rRole );
    xLSeq->setValues( xNewSequence );
    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSequences(
        xDataSource->getDataSequences());
    aSequences.realloc( aSequences.getLength() + 1 );
    auto pSequences = aSequences.getArray();
    pSequences[ aSequences.getLength() - 1 ] = xLSeq;
    xSink->setData( aSequences );
}

void lcl_setXMLRangePropertyAtDataSequence(
    const Reference< chart2::data::XDataSequence > & xDataSequence,
    const OUString & rXMLRange )
{
    try
    {
        static constexpr OUString aXMLRangePropName( u"CachedXMLRange"_ustr);
        Reference< beans::XPropertySet > xProp( xDataSequence, uno::UNO_QUERY_THROW );
        Reference< beans::XPropertySetInfo > xInfo( xProp->getPropertySetInfo());
        if( xInfo.is() && xInfo->hasPropertyByName( aXMLRangePropName ))
            xProp->setPropertyValue( aXMLRangePropName, uno::Any( rXMLRange ));
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

} // anonymous namespace

namespace chart
{

double StatisticsHelper::getVariance(
    const Sequence< double > & rData )
{
    sal_Int32 nValCount;
    return lcl_getVariance( rData, nValCount );
}

double StatisticsHelper::getStandardDeviation( const Sequence< double > & rData )
{
    double fResult = getVariance( rData );
    if( ! std::isnan( fResult ))
        fResult = sqrt( fResult );

    return fResult;
}

double StatisticsHelper::getStandardError( const Sequence< double > & rData )
{
    sal_Int32 nValCount;
    double fVar = lcl_getVariance( rData, nValCount );

    if( nValCount == 0 || std::isnan( fVar ))
        return std::numeric_limits<double>::quiet_NaN();
    // standard-deviation / sqrt(n)
    return sqrt( fVar ) / sqrt( double(nValCount) );
}

uno::Reference< chart2::data::XLabeledDataSequence > StatisticsHelper::getErrorLabeledDataSequenceFromDataSource(
    const Reference< chart2::data::XDataSource > & xDataSource,
    bool bPositiveValue,
    bool bYError /* = true */ )
{
    uno::Reference< chart2::data::XLabeledDataSequence > xResult;
    if( !xDataSource.is())
        return xResult;

    OUString aRole;
    uno::Reference< chart2::data::XLabeledDataSequence > xLSeq =
        lcl_getErrorBarLabeledSequence( xDataSource, bPositiveValue, bYError, aRole );
    if( xLSeq.is())
        xResult = xLSeq;

    return xResult;
}

Reference< chart2::data::XDataSequence > StatisticsHelper::getErrorDataSequenceFromDataSource(
    const Reference< chart2::data::XDataSource > & xDataSource,
    bool bPositiveValue,
    bool bYError /* = true */ )
{
    uno::Reference< chart2::data::XLabeledDataSequence > xLSeq =
        StatisticsHelper::getErrorLabeledDataSequenceFromDataSource(
            xDataSource, bPositiveValue,
            bYError );
    if( !xLSeq.is())
        return Reference< chart2::data::XDataSequence >();

    return xLSeq->getValues();
}

double StatisticsHelper::getErrorFromDataSource(
    const Reference< chart2::data::XDataSource > & xDataSource,
    sal_Int32 nIndex,
    bool bPositiveValue,
    bool bYError /* = true */ )
{
    double fResult = std::numeric_limits<double>::quiet_NaN();

    Reference< chart2::data::XDataSequence > xValues(
        StatisticsHelper::getErrorDataSequenceFromDataSource( xDataSource, bPositiveValue, bYError ));

    Reference< chart2::data::XNumericalDataSequence > xNumValues( xValues, uno::UNO_QUERY );
    if( xNumValues.is())
    {
        Sequence< double > aData( xNumValues->getNumericalData());
        if( nIndex < aData.getLength())
            fResult = aData[nIndex];
    }
    else if( xValues.is())
    {
        Sequence< uno::Any > aData( xValues->getData());
        if( nIndex < aData.getLength())
            aData[nIndex] >>= fResult;
    }

    return fResult;
}

void StatisticsHelper::setErrorDataSequence(
    const Reference< chart2::data::XDataSource > & xDataSource,
    const Reference< chart2::data::XDataProvider > & xDataProvider,
    const OUString & rNewRange,
    bool bPositiveValue,
    bool bYError /* = true */,
    OUString const * pXMLRange /* = 0 */ )
{
    Reference< chart2::data::XDataSink > xDataSink( xDataSource, uno::UNO_QUERY );
    if( ! ( xDataSink.is() && xDataProvider.is()))
        return;

    OUString aRole;
    Reference< chart2::data::XLabeledDataSequence > xLSeq(
        lcl_getErrorBarLabeledSequence( xDataSource, bPositiveValue, bYError, aRole ));
    Reference< chart2::data::XDataSequence > xNewSequence(
        xDataProvider->createDataSequenceByRangeRepresentation( rNewRange ));
    if( xNewSequence.is())
    {
        if( pXMLRange )
            lcl_setXMLRangePropertyAtDataSequence( xNewSequence, *pXMLRange );
        if( xLSeq.is())
        {
            lcl_setRole( xNewSequence, aRole );
            xLSeq->setValues( xNewSequence );
        }
        else
            lcl_addSequenceToDataSource( xDataSource, xNewSequence, aRole );
    }
}

Reference< beans::XPropertySet > StatisticsHelper::addErrorBars(
    const rtl::Reference< DataSeries > & xDataSeries,
    sal_Int32 nStyle,
    bool bYError /* = true */ )
{
    Reference< beans::XPropertySet > xErrorBar;
    if( !xDataSeries.is())
        return xErrorBar;

    const OUString aPropName(
            bYError ? CHART_UNONAME_ERRORBAR_Y : CHART_UNONAME_ERRORBAR_X);
    if( !( xDataSeries->getPropertyValue( aPropName ) >>= xErrorBar ) ||
        !xErrorBar.is())
    {
        xErrorBar.set( new ErrorBar );
    }

    OSL_ASSERT( xErrorBar.is());
    if( xErrorBar.is())
    {
        xErrorBar->setPropertyValue( "ErrorBarStyle", uno::Any( nStyle ));
    }

    xDataSeries->setPropertyValue( aPropName, uno::Any( xErrorBar ));

    return xErrorBar;
}

Reference< beans::XPropertySet > StatisticsHelper::getErrorBars(
    const rtl::Reference< DataSeries > & xDataSeries,
    bool bYError /* = true */ )
{
    Reference< beans::XPropertySet > xErrorBar;
    const OUString aPropName(
            bYError ? CHART_UNONAME_ERRORBAR_Y : CHART_UNONAME_ERRORBAR_X);

    if ( xDataSeries.is())
        xDataSeries->getPropertyValue( aPropName ) >>= xErrorBar;

    return xErrorBar;
}

bool StatisticsHelper::hasErrorBars(
    const rtl::Reference< DataSeries > & xDataSeries,
    bool bYError /* = true */ )
{
    Reference< beans::XPropertySet > xErrorBar( getErrorBars( xDataSeries, bYError ));
    sal_Int32 nStyle = css::chart::ErrorBarStyle::NONE;

    return ( xErrorBar.is() &&
             ( xErrorBar->getPropertyValue( "ErrorBarStyle") >>= nStyle ) &&
             nStyle != css::chart::ErrorBarStyle::NONE );
}

void StatisticsHelper::removeErrorBars(
    const rtl::Reference< DataSeries > & xDataSeries,
    bool bYError /* = true  */ )
{
    Reference< beans::XPropertySet > xErrorBar( getErrorBars( xDataSeries, bYError ));
    if ( xErrorBar.is())
        xErrorBar->setPropertyValue( "ErrorBarStyle", uno::Any(
                                         css::chart::ErrorBarStyle::NONE ));
}

bool StatisticsHelper::usesErrorBarRanges(
    const rtl::Reference< DataSeries > & xDataSeries,
    bool bYError /* = true */ )
{
    Reference< beans::XPropertySet > xErrorBar( getErrorBars( xDataSeries, bYError ));
    sal_Int32 nStyle = css::chart::ErrorBarStyle::NONE;

    return ( xErrorBar.is() &&
             ( xErrorBar->getPropertyValue( "ErrorBarStyle") >>= nStyle ) &&
             nStyle == css::chart::ErrorBarStyle::FROM_DATA );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
