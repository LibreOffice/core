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

#include "RegressionCurveHelper.hxx"
#include "MeanValueRegressionCurveCalculator.hxx"
#include "LinearRegressionCurveCalculator.hxx"
#include "PolynomialRegressionCurveCalculator.hxx"
#include "MovingAverageRegressionCurveCalculator.hxx"
#include "LogarithmicRegressionCurveCalculator.hxx"
#include "ExponentialRegressionCurveCalculator.hxx"
#include "PotentialRegressionCurveCalculator.hxx"
#include "CommonConverters.hxx"
#include "RegressionCurveModel.hxx"
#include "ChartTypeHelper.hxx"
#include "ChartModelHelper.hxx"
#include "macros.hxx"
#include "PropertyHelper.hxx"
#include "ResId.hxx"
#include "Strings.hrc"
#include "DiagramHelper.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::lang::XServiceName;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Exception;

namespace
{
OUString lcl_getServiceNameForType(SvxChartRegress eType)
{
    OUString aServiceName;
    switch( eType )
    {
        case SvxChartRegress::Linear:
            aServiceName = "com.sun.star.chart2.LinearRegressionCurve";
            break;
        case SvxChartRegress::Log:
            aServiceName = "com.sun.star.chart2.LogarithmicRegressionCurve";
            break;
        case SvxChartRegress::Exp:
            aServiceName = "com.sun.star.chart2.ExponentialRegressionCurve";
            break;
        case SvxChartRegress::Power:
            aServiceName = "com.sun.star.chart2.PotentialRegressionCurve";
            break;
        case SvxChartRegress::Polynomial:
            aServiceName = "com.sun.star.chart2.PolynomialRegressionCurve";
            break;
        case SvxChartRegress::MovingAverage:
            aServiceName = "com.sun.star.chart2.MovingAverageRegressionCurve";
            break;
        default:
            OSL_FAIL("unknown regression curve type - use linear instead");
            aServiceName = "com.sun.star.chart2.LinearRegressionCurve";
            break;
    }
    return aServiceName;
}

} // anonymous namespace

namespace chart
{

Reference< XRegressionCurve > RegressionCurveHelper::createMeanValueLine()
{
    return Reference< XRegressionCurve >( new MeanValueRegressionCurve );
}

Reference< XRegressionCurve > RegressionCurveHelper::createRegressionCurveByServiceName(
    const OUString& aServiceName )
{
    Reference< XRegressionCurve > xResult;

    // todo: use factory methods with service name
    if( aServiceName == "com.sun.star.chart2.LinearRegressionCurve" )
    {
        xResult.set( new LinearRegressionCurve );
    }
    else if( aServiceName == "com.sun.star.chart2.LogarithmicRegressionCurve" )
    {
        xResult.set( new LogarithmicRegressionCurve );
    }
    else if( aServiceName == "com.sun.star.chart2.ExponentialRegressionCurve" )
    {
        xResult.set( new ExponentialRegressionCurve );
    }
    else if( aServiceName == "com.sun.star.chart2.PotentialRegressionCurve" )
    {
        xResult.set( new PotentialRegressionCurve );
    }
    else if( aServiceName == "com.sun.star.chart2.PolynomialRegressionCurve" )
    {
        xResult.set( new PolynomialRegressionCurve );
    }
    else if( aServiceName == "com.sun.star.chart2.MovingAverageRegressionCurve" )
    {
        xResult.set( new MovingAverageRegressionCurve );
    }

    return xResult;
}

Reference< XRegressionCurveCalculator > RegressionCurveHelper::createRegressionCurveCalculatorByServiceName(
    const OUString& aServiceName )
{
    Reference< XRegressionCurveCalculator > xResult;

    // todo: use factory methods with service name
    if( aServiceName == "com.sun.star.chart2.MeanValueRegressionCurve" )
    {
        xResult.set( new MeanValueRegressionCurveCalculator() );
    }
    if( aServiceName == "com.sun.star.chart2.LinearRegressionCurve" )
    {
        xResult.set( new LinearRegressionCurveCalculator() );
    }
    else if( aServiceName == "com.sun.star.chart2.LogarithmicRegressionCurve" )
    {
        xResult.set( new LogarithmicRegressionCurveCalculator() );
    }
    else if( aServiceName == "com.sun.star.chart2.ExponentialRegressionCurve" )
    {
        xResult.set( new ExponentialRegressionCurveCalculator() );
    }
    else if( aServiceName == "com.sun.star.chart2.PotentialRegressionCurve" )
    {
        xResult.set( new PotentialRegressionCurveCalculator() );
    }
    else if( aServiceName == "com.sun.star.chart2.PolynomialRegressionCurve" )
    {
        xResult.set( new PolynomialRegressionCurveCalculator() );
    }
    else if( aServiceName == "com.sun.star.chart2.MovingAverageRegressionCurve" )
    {
        xResult.set( new MovingAverageRegressionCurveCalculator() );
    }

    return xResult;
}

void RegressionCurveHelper::initializeCurveCalculator(
    const Reference< XRegressionCurveCalculator > & xOutCurveCalculator,
    const Reference< data::XDataSource > & xSource,
    bool bUseXValuesIfAvailable /* = true */ )
{
    if( ! (xOutCurveCalculator.is() &&
           xSource.is() ))
        return;

    Sequence< double > aXValues, aYValues;
    bool bXValuesFound = false, bYValuesFound = false;

    Sequence< Reference< data::XLabeledDataSequence > > aDataSeqs( xSource->getDataSequences());
    sal_Int32 i = 0;
    for( i=0;
         ! (bXValuesFound && bYValuesFound) && i<aDataSeqs.getLength();
         ++i )
    {
        try
        {
            Reference< data::XDataSequence > xSeq( aDataSeqs[i]->getValues());
            Reference< XPropertySet > xProp( xSeq, uno::UNO_QUERY_THROW );
            OUString aRole;
            if( xProp->getPropertyValue( "Role" ) >>= aRole )
            {
                if( bUseXValuesIfAvailable && !bXValuesFound && aRole == "values-x" )
                {
                    aXValues = DataSequenceToDoubleSequence( xSeq );
                    bXValuesFound = true;
                }
                else if( !bYValuesFound && aRole == "values-y" )
                {
                    aYValues = DataSequenceToDoubleSequence( xSeq );
                    bYValuesFound = true;
                }
            }
        }
        catch( const Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    if( ! bXValuesFound &&
        bYValuesFound )
    {
        // initialize with 1, 2, ...
        //first category (index 0) matches with real number 1.0
        aXValues.realloc( aYValues.getLength());
        for( i=0; i<aXValues.getLength(); ++i )
            aXValues[i] = i+1;
        bXValuesFound = true;
    }

    if( bXValuesFound && bYValuesFound &&
        aXValues.getLength() > 0 &&
        aYValues.getLength() > 0 )
        xOutCurveCalculator->recalculateRegression( aXValues, aYValues );
}

void RegressionCurveHelper::initializeCurveCalculator(
    const Reference< XRegressionCurveCalculator > & xOutCurveCalculator,
    const Reference< XDataSeries > & xSeries,
    const Reference< frame::XModel > & xModel )
{
    sal_Int32 nAxisType = ChartTypeHelper::getAxisType(
        ChartModelHelper::getChartTypeOfSeries( xModel, xSeries ), 0 ); // x-axis

    initializeCurveCalculator( xOutCurveCalculator,
                               uno::Reference< data::XDataSource >( xSeries, uno::UNO_QUERY ),
                               (nAxisType == AxisType::REALNUMBER) );
}

bool RegressionCurveHelper::hasMeanValueLine(
    const uno::Reference< XRegressionCurveContainer > & xRegCnt )
{
    if( !xRegCnt.is())
        return false;

    try
    {
        uno::Sequence< uno::Reference< XRegressionCurve > > aCurves(
            xRegCnt->getRegressionCurves());
        for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
        {
            if( isMeanValueLine( aCurves[i] ))
                return true;
        }
    }
    catch( const Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return false;
}

bool RegressionCurveHelper::isMeanValueLine(
    const uno::Reference< chart2::XRegressionCurve > & xRegCurve )
{
    uno::Reference< XServiceName > xServName( xRegCurve, uno::UNO_QUERY );
    if( xServName.is() &&
        xServName->getServiceName() ==
            "com.sun.star.chart2.MeanValueRegressionCurve" )
        return true;
    return false;
}

uno::Reference< chart2::XRegressionCurve >
    RegressionCurveHelper::getMeanValueLine(
        const uno::Reference< chart2::XRegressionCurveContainer > & xRegCnt )
{
    if( xRegCnt.is())
    {
        try
        {
            uno::Sequence< uno::Reference< XRegressionCurve > > aCurves(
                xRegCnt->getRegressionCurves());
            for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
            {
                if( isMeanValueLine( aCurves[i] ))
                    return aCurves[i];
            }
        }
        catch( const Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return uno::Reference< chart2::XRegressionCurve >();
}

void RegressionCurveHelper::addMeanValueLine(
    uno::Reference< XRegressionCurveContainer > & xRegCnt,
    const uno::Reference< XPropertySet > & xSeriesProp )
{
    if( !xRegCnt.is() ||
        ::chart::RegressionCurveHelper::hasMeanValueLine( xRegCnt ) )
        return;

    // todo: use a valid context
    uno::Reference< XRegressionCurve > xCurve( createMeanValueLine() );
    xRegCnt->addRegressionCurve( xCurve );

    if( xSeriesProp.is())
    {
        uno::Reference< XPropertySet > xProp( xCurve, uno::UNO_QUERY );
        if( xProp.is())
        {
            xProp->setPropertyValue( "LineColor",
                                     xSeriesProp->getPropertyValue( "Color"));
        }
    }
}

void RegressionCurveHelper::removeMeanValueLine(
    Reference< XRegressionCurveContainer > & xRegCnt )
{
    if( !xRegCnt.is())
        return;

    try
    {
        Sequence< Reference< XRegressionCurve > > aCurves(
            xRegCnt->getRegressionCurves());
        for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
        {
            if( isMeanValueLine( aCurves[i] ))
            {
                xRegCnt->removeRegressionCurve( aCurves[i] );
                // attention: the iterator i has become invalid now

                // note: assume that there is only one mean-value curve
                // to remove multiple mean-value curves remove the break
                break;
            }
        }
    }
    catch( const Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

uno::Reference< chart2::XRegressionCurve > RegressionCurveHelper::addRegressionCurve(
    SvxChartRegress eType,
    uno::Reference< XRegressionCurveContainer >& xRegressionCurveContainer,
    const uno::Reference< XComponentContext >& /* xContext */,
    const uno::Reference< beans::XPropertySet >& xPropertySource,
    const uno::Reference< beans::XPropertySet >& xEquationProperties )
{
    uno::Reference< chart2::XRegressionCurve > xCurve;

    if( !xRegressionCurveContainer.is() )
        return xCurve;

    if( eType == SvxChartRegress::NONE )
    {
        OSL_FAIL("don't create a regression curve of type none");
        return xCurve;
    }

    OUString aServiceName( lcl_getServiceNameForType( eType ));
    if( !aServiceName.isEmpty())
    {
        // todo: use a valid context
        xCurve.set( createRegressionCurveByServiceName( aServiceName ) );

        if( xEquationProperties.is())
            xCurve->setEquationProperties( xEquationProperties );

        uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
        if( xProperties.is())
        {
            if( xPropertySource.is())
                comphelper::copyProperties( xPropertySource, xProperties );
            else
            {
                uno::Reference< XPropertySet > xSeriesProp( xRegressionCurveContainer, uno::UNO_QUERY );
                if( xSeriesProp.is())
                {
                    xProperties->setPropertyValue( "LineColor",
                                             xSeriesProp->getPropertyValue( "Color"));
                }
            }
        }
    }
    xRegressionCurveContainer->addRegressionCurve( xCurve );

    return xCurve;
}

/** removes all regression curves that are not of type mean value
    and returns true, if anything was removed
 */
bool RegressionCurveHelper::removeAllExceptMeanValueLine(
    uno::Reference< chart2::XRegressionCurveContainer > & xRegCnt )
{
    bool bRemovedSomething = false;
    if( xRegCnt.is())
    {
        try
        {
            uno::Sequence< uno::Reference< chart2::XRegressionCurve > > aCurves(
                xRegCnt->getRegressionCurves());
            ::std::vector< uno::Reference< chart2::XRegressionCurve > > aCurvesToDelete;
            for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
            {
                if( ! isMeanValueLine( aCurves[i] ))
                {
                    aCurvesToDelete.push_back( aCurves[ i ] );
                }
            }

            for( ::std::vector< uno::Reference< chart2::XRegressionCurve > >::const_iterator aIt = aCurvesToDelete.begin();
                     aIt != aCurvesToDelete.end(); ++aIt )
            {
                xRegCnt->removeRegressionCurve( *aIt );
                bRemovedSomething = true;
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
    return bRemovedSomething;
}

void RegressionCurveHelper::removeEquations(
        uno::Reference< chart2::XRegressionCurveContainer > & xRegCnt )
{
    if( xRegCnt.is())
    {
        try
        {
            uno::Sequence< uno::Reference< chart2::XRegressionCurve > > aCurves(
                xRegCnt->getRegressionCurves());
            for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
            {
                if( !isMeanValueLine( aCurves[i] ) )
                {
                    uno::Reference< chart2::XRegressionCurve > xRegCurve( aCurves[ i ] );
                    if( xRegCurve.is() )
                    {
                        uno::Reference< beans::XPropertySet > xEqProp( xRegCurve->getEquationProperties() ) ;
                        if( xEqProp.is())
                        {
                            xEqProp->setPropertyValue( "ShowEquation", uno::Any( false ));
                            xEqProp->setPropertyValue( "XName", uno::Any( OUString("x") ));
                            xEqProp->setPropertyValue( "YName", uno::Any( OUString("f(x) ") ));
                            xEqProp->setPropertyValue( "ShowCorrelationCoefficient", uno::Any( false ));
                        }
                    }
                }
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

uno::Reference< XRegressionCurve > RegressionCurveHelper::changeRegressionCurveType(
    SvxChartRegress eType,
    uno::Reference< XRegressionCurveContainer > & xRegressionCurveContainer,
    uno::Reference< XRegressionCurve > & xRegressionCurve,
    const uno::Reference< XComponentContext > & xContext )
{
    xRegressionCurveContainer->removeRegressionCurve( xRegressionCurve );
    return RegressionCurveHelper::addRegressionCurve(
            eType,
            xRegressionCurveContainer,
            xContext,
            uno::Reference< beans::XPropertySet >( xRegressionCurve, uno::UNO_QUERY ),
            xRegressionCurve->getEquationProperties());
}

uno::Reference< chart2::XRegressionCurve > RegressionCurveHelper::getFirstCurveNotMeanValueLine(
    const Reference< XRegressionCurveContainer > & xRegCnt )
{
    if( !xRegCnt.is())
        return nullptr;

    try
    {
        uno::Sequence< uno::Reference< chart2::XRegressionCurve > > aCurves(
            xRegCnt->getRegressionCurves());
        for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
        {
            if( ! isMeanValueLine( aCurves[i] ))
            {
                return aCurves[ i ];
            }
        }
    }
    catch( const Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return nullptr;
}

uno::Reference< chart2::XRegressionCurve > RegressionCurveHelper::getRegressionCurveAtIndex(
    const Reference< XRegressionCurveContainer >& xCurveContainer,
    sal_Int32 aIndex )
{
    if( !xCurveContainer.is())
        return nullptr;

    try
    {
        uno::Sequence< uno::Reference< chart2::XRegressionCurve > > aCurves(xCurveContainer->getRegressionCurves());
        if(0 <= aIndex && aIndex < aCurves.getLength())
        {
            if(!isMeanValueLine(aCurves[aIndex]))
                return aCurves[aIndex];
        }
    }
    catch( const Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return nullptr;
}

SvxChartRegress RegressionCurveHelper::getRegressionType(
    const Reference< XRegressionCurve > & xCurve )
{
    SvxChartRegress eResult = SvxChartRegress::Unknown;

    try
    {
        Reference< lang::XServiceName > xServName( xCurve, uno::UNO_QUERY );
        if( xServName.is())
        {
            OUString aServiceName( xServName->getServiceName() );

            if( aServiceName == "com.sun.star.chart2.LinearRegressionCurve" )
            {
                eResult = SvxChartRegress::Linear;
            }
            else if( aServiceName == "com.sun.star.chart2.LogarithmicRegressionCurve" )
            {
                eResult = SvxChartRegress::Log;
            }
            else if( aServiceName == "com.sun.star.chart2.ExponentialRegressionCurve" )
            {
                eResult = SvxChartRegress::Exp;
            }
            else if( aServiceName == "com.sun.star.chart2.PotentialRegressionCurve" )
            {
                eResult = SvxChartRegress::Power;
            }
            else if( aServiceName == "com.sun.star.chart2.MeanValueRegressionCurve" )
            {
                eResult = SvxChartRegress::MeanValue;
            }
            else if( aServiceName == "com.sun.star.chart2.PolynomialRegressionCurve" )
            {
                eResult = SvxChartRegress::Polynomial;
            }
            else if( aServiceName == "com.sun.star.chart2.MovingAverageRegressionCurve" )
            {
                eResult = SvxChartRegress::MovingAverage;
            }
        }
    }
    catch( const Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return eResult;
}

SvxChartRegress RegressionCurveHelper::getFirstRegressTypeNotMeanValueLine(
    const Reference< XRegressionCurveContainer > & xRegCnt )
{
    SvxChartRegress eResult = SvxChartRegress::NONE;

    if( xRegCnt.is())
    {
        Sequence< Reference< XRegressionCurve > > aCurves(
            xRegCnt->getRegressionCurves());
        for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
        {
            SvxChartRegress eType = getRegressionType( aCurves[i] );
            if( eType != SvxChartRegress::MeanValue &&
                eType != SvxChartRegress::Unknown )
            {
                eResult = eType;
                break;
            }
        }
    }

    return eResult;
}

OUString RegressionCurveHelper::getUINameForRegressionCurve( const Reference< XRegressionCurve >& xRegressionCurve )
{
    OUString aResult = getRegressionCurveSpecificName(xRegressionCurve);
    if (aResult.isEmpty())
    {
        aResult = getRegressionCurveGenericName(xRegressionCurve);
        if (!aResult.isEmpty())
        {
            aResult += " (%SERIESNAME)";
        }
    }
    return aResult;
}

OUString RegressionCurveHelper::getRegressionCurveGenericName(const Reference< XRegressionCurve >& xRegressionCurve)
{
    OUString aResult;
    if(!xRegressionCurve.is())
        return aResult;

    Reference< lang::XServiceName > xServiceName( xRegressionCurve, uno::UNO_QUERY );
    if(!xServiceName.is())
        return aResult;

    OUString aServiceName(xServiceName->getServiceName());

    if( aServiceName == "com.sun.star.chart2.MeanValueRegressionCurve" )
    {
        aResult = SCH_RESSTR(STR_REGRESSION_MEAN);
    }
    else if( aServiceName == "com.sun.star.chart2.LinearRegressionCurve" )
    {
        aResult = SCH_RESSTR(STR_REGRESSION_LINEAR);
    }
    else if( aServiceName == "com.sun.star.chart2.LogarithmicRegressionCurve" )
    {
        aResult = SCH_RESSTR(STR_REGRESSION_LOG);
    }
    else if( aServiceName == "com.sun.star.chart2.ExponentialRegressionCurve" )
    {
        aResult = SCH_RESSTR(STR_REGRESSION_EXP);
    }
    else if( aServiceName == "com.sun.star.chart2.PotentialRegressionCurve" )
    {
        aResult = SCH_RESSTR(STR_REGRESSION_POWER);
    }
    else if( aServiceName == "com.sun.star.chart2.PolynomialRegressionCurve" )
    {
        aResult = SCH_RESSTR(STR_REGRESSION_POLYNOMIAL);
    }
    else if( aServiceName == "com.sun.star.chart2.MovingAverageRegressionCurve" )
    {
        aResult = SCH_RESSTR(STR_REGRESSION_MOVING_AVERAGE);
    }
    return aResult;
}

OUString RegressionCurveHelper::getRegressionCurveSpecificName(const Reference< XRegressionCurve >& xRegressionCurve)
{
    OUString aResult;

    if(!xRegressionCurve.is())
        return aResult;

    Reference<XPropertySet> xProperties( xRegressionCurve, uno::UNO_QUERY );
    if(!xProperties.is())
        return aResult;

    xProperties->getPropertyValue("CurveName") >>= aResult;

    return aResult;
}

OUString RegressionCurveHelper::getRegressionCurveName( const Reference< XRegressionCurve >& xRegressionCurve )
{
    OUString aResult = getRegressionCurveSpecificName(xRegressionCurve);
    if (aResult.isEmpty())
        return getRegressionCurveGenericName(xRegressionCurve);
    return aResult;
}

std::vector< Reference< chart2::XRegressionCurve > >
    RegressionCurveHelper::getAllRegressionCurvesNotMeanValueLine(
        const Reference< chart2::XDiagram > & xDiagram )
{
    std::vector< Reference< chart2::XRegressionCurve > > aResult;
    std::vector< Reference< chart2::XDataSeries > > aSeries( DiagramHelper::getDataSeriesFromDiagram( xDiagram ));
    for( std::vector< Reference< chart2::XDataSeries > >::iterator aIt( aSeries.begin());
         aIt != aSeries.end(); ++aIt )
    {
        Reference< chart2::XRegressionCurveContainer > xContainer( *aIt, uno::UNO_QUERY );
        if(xContainer.is())
        {
            uno::Sequence< uno::Reference< chart2::XRegressionCurve > > aCurves(xContainer->getRegressionCurves());
            for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
            {
                if( ! isMeanValueLine( aCurves[i] ))
                    aResult.push_back( aCurves[i] );
            }
        }
    }

    return aResult;
}

void RegressionCurveHelper::resetEquationPosition(
    const Reference< chart2::XRegressionCurve > & xCurve )
{
    if( xCurve.is())
    {
        try
        {
            const OUString aPosPropertyName( "RelativePosition" );
            Reference< beans::XPropertySet > xEqProp( xCurve->getEquationProperties()); // since m233: , uno::UNO_SET_THROW );
            if( xEqProp->getPropertyValue( aPosPropertyName ).hasValue())
                xEqProp->setPropertyValue( aPosPropertyName, uno::Any());
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

sal_Int32 RegressionCurveHelper::getRegressionCurveIndex(
    const Reference< chart2::XRegressionCurveContainer >& xContainer,
    const Reference< chart2::XRegressionCurve >& xCurve )
{
    if( xContainer.is())
    {
        uno::Sequence< uno::Reference< XRegressionCurve > > aCurves(
            xContainer->getRegressionCurves());

        for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
        {
            if( xCurve == aCurves[i] )
                return i;
        }
    }
    return -1;
}

bool RegressionCurveHelper::hasEquation( const Reference< chart2::XRegressionCurve > & xCurve )
{
    bool bHasEquation = false;
    if( xCurve.is())
    {
        uno::Reference< beans::XPropertySet > xEquationProp( xCurve->getEquationProperties());
        if( xEquationProp.is())
        {
            bool bShowEquation = false;
            bool bShowCoefficient = false;
            xEquationProp->getPropertyValue( "ShowEquation") >>= bShowEquation;
            xEquationProp->getPropertyValue( "ShowCorrelationCoefficient") >>= bShowCoefficient;
            bHasEquation = bShowEquation || bShowCoefficient;
        }
    }
    return bHasEquation;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
