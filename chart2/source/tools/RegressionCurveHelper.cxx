/*************************************************************************
 *
 *  $RCSfile: RegressionCurveHelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: iha $ $Date: 2003-12-17 17:43:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <drafts/com/sun/star/chart2/XChartDocument.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::XComponentContext;

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
    for( sal_Int32 i=0;
         ! (bXValuesFound && bYValuesFound) && i<aDataSeqs.getLength();
         ++i )
    {
        try
        {
            Reference< beans::XPropertySet > xProp( aDataSeqs[i], uno::UNO_QUERY_THROW );
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
                         aRole.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "values" )))
                {
                    aYValues = DataSequenceToDoubleSequence( aDataSeqs[i] );
                    bYValuesFound = true;
                }
            }
        }
        catch( uno::Exception )
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

//.............................................................................
} //namespace chart
//.............................................................................
