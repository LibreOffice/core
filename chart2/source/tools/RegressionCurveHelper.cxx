/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RegressionCurveHelper.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:33:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "RegressionCurveHelper.hxx"
#include "ContextHelper.hxx"
#include "MeanValueRegressionCurveCalculator.hxx"
#include "LinearRegressionCurveCalculator.hxx"
#include "LogarithmicRegressionCurveCalculator.hxx"
#include "ExponentialRegressionCurveCalculator.hxx"
#include "PotentialRegressionCurveCalculator.hxx"
#include "CommonConverters.hxx"
#include "RegressionCurveModel.hxx"
#include "ChartTypeHelper.hxx"
#include "ChartModelHelper.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::lang::XServiceName;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Exception;

// static
Reference< XRegressionCurve > RegressionCurveHelper::createMeanValueLine(
    const Reference< XComponentContext > & xContext )
{
    ContextHelper::tContextEntryMapType aContextValues(
        ContextHelper::MakeContextEntryMap(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Identifier" )), uno::makeAny(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "@mean-value-line" )))));

    return Reference< XRegressionCurve >(
        new RegressionCurveModel( ContextHelper::createContext( aContextValues, xContext ),
                                  RegressionCurveModel::CURVE_TYPE_MEAN_VALUE ));
}

// static
Reference< XRegressionCurve > RegressionCurveHelper::createRegressionCurveByServiceName(
    const Reference< XComponentContext > & xContext,
    ::rtl::OUString aServiceName )
{
    Reference< XRegressionCurve > xResult;

    // todo: use factory methods with service name
    if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                "com.sun.star.chart2.LinearRegressionCurve" )))
    {
        ContextHelper::tContextEntryMapType aContextValues(
            ContextHelper::MakeContextEntryMap(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Identifier" )), uno::makeAny(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "@linear-regression" )))));

        xResult.set(
            new RegressionCurveModel( ContextHelper::createContext( aContextValues, xContext ),
                                      RegressionCurveModel::CURVE_TYPE_LINEAR ));
    }
    else if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                     "com.sun.star.chart2.LogarithmicRegressionCurve" )))
    {
        ContextHelper::tContextEntryMapType aContextValues(
            ContextHelper::MakeContextEntryMap(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Identifier" )), uno::makeAny(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "@logarithmic-regression" )))));

        xResult.set(
            new RegressionCurveModel( ContextHelper::createContext( aContextValues, xContext ),
                                      RegressionCurveModel::CURVE_TYPE_LOGARITHM ));
    }
    else if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                     "com.sun.star.chart2.ExponentialRegressionCurve" )))
    {
        ContextHelper::tContextEntryMapType aContextValues(
            ContextHelper::MakeContextEntryMap(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Identifier" )), uno::makeAny(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "@exponential-regression" )))));

        xResult.set(
            new RegressionCurveModel( ContextHelper::createContext( aContextValues, xContext ),
                                      RegressionCurveModel::CURVE_TYPE_EXPONENTIAL ));
    }
    else if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                     "com.sun.star.chart2.PotentialRegressionCurve" )))
    {
        ContextHelper::tContextEntryMapType aContextValues(
            ContextHelper::MakeContextEntryMap(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Identifier" )), uno::makeAny(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "@potential-regression" )))));

        xResult.set(
            new RegressionCurveModel( ContextHelper::createContext( aContextValues, xContext ),
                                      RegressionCurveModel::CURVE_TYPE_POWER ));
    }

    return xResult;
}

// ------------------------------------------------------------

// static
Reference< XRegressionCurveCalculator > RegressionCurveHelper::createMeanValueLineCalculator()
{
    return Reference< XRegressionCurveCalculator >( new MeanValueRegressionCurveCalculator());
}

// static
Reference< XRegressionCurveCalculator > RegressionCurveHelper::createRegressionCurveCalculatorByServiceName(
    ::rtl::OUString aServiceName )
{
    Reference< XRegressionCurveCalculator > xResult;

    // todo: use factory methods with service name
    if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                     "com.sun.star.chart2.MeanValueRegressionCurve" )))
    {
        xResult.set( new MeanValueRegressionCurveCalculator());
    }
    if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                     "com.sun.star.chart2.LinearRegressionCurve" )))
    {
        xResult.set( new LinearRegressionCurveCalculator());
    }
    else if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                     "com.sun.star.chart2.LogarithmicRegressionCurve" )))
    {
        xResult.set( new LogarithmicRegressionCurveCalculator());
    }
    else if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                     "com.sun.star.chart2.ExponentialRegressionCurve" )))
    {
        xResult.set( new ExponentialRegressionCurveCalculator());
    }
    else if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                     "com.sun.star.chart2.PotentialRegressionCurve" )))
    {
        xResult.set( new PotentialRegressionCurveCalculator());
    }

    return xResult;
}

// static
void RegressionCurveHelper::initializeCurveCalculator(
    const Reference< XRegressionCurveCalculator > & xOutCurveCalculator,
    const Reference< XDataSource > & xSource,
    bool bUseXValuesIfAvailable /* = true */ )
{
    if( ! (xOutCurveCalculator.is() &&
           xSource.is() ))
        return;

    Sequence< double > aXValues, aYValues;
    bool bXValuesFound = false, bYValuesFound = false;

    Sequence< Reference< XDataSequence > > aDataSeqs( xSource->getDataSequences());
    sal_Int32 i = 0;
    for( i=0;
         ! (bXValuesFound && bYValuesFound) && i<aDataSeqs.getLength();
         ++i )
    {
        try
        {
            Reference< XPropertySet > xProp( aDataSeqs[i], uno::UNO_QUERY_THROW );
            ::rtl::OUString aRole;
            if( xProp->getPropertyValue(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Role" ))) >>= aRole )
            {
                if( bUseXValuesIfAvailable &&
                    ! bXValuesFound &&
                    aRole.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "x-values" )))
                {
                    aXValues = DataSequenceToDoubleSequence( aDataSeqs[i] );
                    bXValuesFound = true;
                }
                else if( ! bYValuesFound &&
                         aRole.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "y-values" )))
                {
                    aYValues = DataSequenceToDoubleSequence( aDataSeqs[i] );
                    bYValuesFound = true;
                }
            }
        }
        catch( Exception )
        {
        }
    }

    if( ! bXValuesFound &&
        bYValuesFound )
    {
        // initialize with 1, 2, ...
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

// static
void RegressionCurveHelper::initializeCurveCalculator(
    const Reference< XRegressionCurveCalculator > & xOutCurveCalculator,
    const Reference< XDataSeries > & xSeries,
    const Reference< frame::XModel > & xModel )
{
    AxisType eAxisType = ChartTypeHelper::getAxisType(
        ChartModelHelper::getChartTypeOfSeries( xModel, xSeries ), 0 ); // x-axis

    initializeCurveCalculator( xOutCurveCalculator,
                               uno::Reference< XDataSource >( xSeries, uno::UNO_QUERY ),
                               (eAxisType == AxisType_REALNUMBER) );
}

// ----------------------------------------

// static
bool RegressionCurveHelper::hasMeanValueLine(
    const uno::Reference< XRegressionCurveContainer > & xRegCnt )
{
    bool bResult = false;

    if( !xRegCnt.is())
        return bResult;

    try
    {
        uno::Sequence< uno::Reference< XRegressionCurve > > aCurves(
            xRegCnt->getRegressionCurves());
        for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
        {
            uno::Reference< XServiceName > xServName( aCurves[i], uno::UNO_QUERY );
            if( xServName.is() &&
                xServName->getServiceName().equals(
                    C2U( "com.sun.star.chart2.MeanValueRegressionCurve" )))
            {
                bResult = true;
                break;
            }
        }
    }
    catch( Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return bResult;
}

// static
void RegressionCurveHelper::addMeanValueLine(
    uno::Reference< XRegressionCurveContainer > & xRegCnt,
    const uno::Reference< XComponentContext > & xContext,
    const uno::Reference< XPropertySet > & xSeriesProp )
{
    if( !xRegCnt.is() ||
        ::chart::RegressionCurveHelper::hasMeanValueLine( xRegCnt ) )
        return;

    // todo: use a valid context
    uno::Reference< XRegressionCurve > xCurve( createMeanValueLine( xContext ));
    xRegCnt->addRegressionCurve( xCurve );

    if( xSeriesProp.is())
    {
        uno::Reference< XPropertySet > xProp( xCurve, uno::UNO_QUERY );
        if( xProp.is())
        {
            xProp->setPropertyValue( C2U( "LineColor" ),
                                     xSeriesProp->getPropertyValue( C2U( "Color" )));
        }
    }
}

// static
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
            Reference< XServiceName > xServName( aCurves[i], uno::UNO_QUERY );
            if( xServName.is() &&
                xServName->getServiceName().equals(
                    C2U( "com.sun.star.chart2.MeanValueRegressionCurve" )))
            {
                xRegCnt->removeRegressionCurve( aCurves[i] );
                // attention: the iterator i has become invalid now

                // note: assume that there is only one mean-value curve
                // to remove multiple mean-value curves remove the break
                break;
            }
        }
    }
    catch( Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// static
RegressionCurveHelper::tRegressionType RegressionCurveHelper::getRegressType(
    const Reference< XRegressionCurveContainer > & xRegCnt )
{
    tRegressionType eResult = REGRESSION_TYPE_NONE;

    if( xRegCnt.is())
    {
        try
        {
            Sequence< Reference< XRegressionCurve > > aCurves(
                xRegCnt->getRegressionCurves());
            for( sal_Int32 i = 0; i < aCurves.getLength(); ++i )
            {
                Reference< lang::XServiceName > xServName( aCurves[i], uno::UNO_QUERY );
                if( xServName.is())
                {
                    ::rtl::OUString aServiceName( xServName->getServiceName() );

                    // note: take first regression curve that matches any known
                    // type (except mean-value line)
                    if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                "com.sun.star.chart2.LinearRegressionCurve" )))
                    {
                        eResult = REGRESSION_TYPE_LINEAR;
                        break;
                    }
                    else if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                "com.sun.star.chart2.LogarithmicRegressionCurve" )))
                    {
                        eResult = REGRESSION_TYPE_LOG;
                        break;
                    }
                    else if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                "com.sun.star.chart2.ExponentialRegressionCurve" )))
                    {
                        eResult = REGRESSION_TYPE_EXP;
                        break;
                    }
                    else if( aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                "com.sun.star.chart2.PotentialRegressionCurve" )))
                    {
                        eResult = REGRESSION_TYPE_POWER;
                        break;
                    }
                }
            }
        }
        catch( Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return eResult;
}

//.............................................................................
} //namespace chart
//.............................................................................
