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

#include <sal/config.h>

#include <string_view>

#include <ObjectNameProvider.hxx>
#include <ResId.hxx>
#include <strings.hrc>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <ChartModel.hxx>
#include <ChartType.hxx>
#include <Diagram.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <TitleHelper.hxx>
#include <ExplicitCategoriesProvider.hxx>
#include <CommonConverters.hxx>
#include <NumberFormatterWrapper.hxx>
#include <RegressionCurveHelper.hxx>
#include <BaseCoordinateSystem.hxx>
#include <RegressionCurveModel.hxx>
#include <rtl/math.hxx>
#include <rtl/ustring.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <unotools/localedatawrapper.hxx>

#include <com/sun/star/chart2/MovingAverageType.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <o3tl/string_view.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

namespace
{

OUString lcl_getDataSeriesName( std::u16string_view rObjectCID, const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    OUString aRet;

    rtl::Reference< Diagram > xDiagram( xChartModel->getFirstChartDiagram() );
    rtl::Reference< DataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( rObjectCID , xChartModel );
    if( xDiagram.is() && xSeries.is() )
    {
        rtl::Reference< ChartType > xChartType( xDiagram->getChartTypeOfSeries( xSeries ) );
        if( xChartType.is() )
        {
            aRet = xSeries->getLabelForRole(
                    xChartType->getRoleOfSequenceForSeriesLabel() ) ;
        }
    }

    return aRet;
}

OUString lcl_getFullSeriesName( std::u16string_view rObjectCID, const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    OUString aRet(SchResId(STR_TIP_DATASERIES));
    OUString aWildcard( u"%SERIESNAME"_ustr );
    sal_Int32 nIndex = aRet.indexOf( aWildcard );
    if( nIndex != -1 )
        aRet = aRet.replaceAt( nIndex, aWildcard.getLength(), lcl_getDataSeriesName( rObjectCID, xChartModel ) );
    return aRet;
}

void lcl_addText( OUString& rOut, std::u16string_view rSeparator, std::u16string_view rNext )
{
    if( !(rOut.isEmpty() || rNext.empty()) )
        rOut+=rSeparator;
    if( !rNext.empty() )
        rOut+=rNext;
}

OUString lcl_getDataPointValueText( const rtl::Reference< DataSeries >& xSeries, sal_Int32 nPointIndex,
                                    const rtl::Reference< BaseCoordinateSystem >& xCooSys,
                                    const Reference< frame::XModel >& xChartModel )
{

    OUString aRet;

    if(!xSeries.is())
        return aRet;

    const std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > & aDataSequences = xSeries->getDataSequences2();

    OUString aX, aY, aY_Min, aY_Max, aY_First, aY_Last, a_Size;
    double fValue = 0;

    uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( xChartModel, uno::UNO_QUERY );
    NumberFormatterWrapper aNumberFormatterWrapper( xNumberFormatsSupplier );
    Color nLabelColor;//dummy
    bool bColorChanged;//dummy

    for(sal_Int32 nN = aDataSequences.size();nN--;)
    {
        uno::Reference<data::XDataSequence>  xDataSequence( aDataSequences[nN]->getValues());
        if( !xDataSequence.is() )
            continue;

        try
        {
            Sequence< Any > aData( xDataSequence->getData() );

            if( nPointIndex >= aData.getLength() )
                continue;
            uno::Reference<beans::XPropertySet> xProp(xDataSequence, uno::UNO_QUERY );
            if( xProp.is())
            {
                uno::Any aARole = xProp->getPropertyValue( u"Role"_ustr );
                OUString aRole;
                aARole >>= aRole;

                if( aRole == "values-x" )
                {
                    aData[nPointIndex]>>= fValue;
                    sal_Int32 nNumberFormatKey = xDataSequence->getNumberFormatKeyByIndex( nPointIndex );
                    aX = aNumberFormatterWrapper.getFormattedString( nNumberFormatKey, fValue, nLabelColor, bColorChanged );
                }
                else if( aRole == "values-y")
                {
                    aData[nPointIndex]>>= fValue;
                    sal_Int32 nNumberFormatKey = xDataSequence->getNumberFormatKeyByIndex( nPointIndex );
                    aY = aNumberFormatterWrapper.getFormattedString( nNumberFormatKey, fValue, nLabelColor, bColorChanged );
                }
                else if( aRole == "values-first" )
                {
                    aData[nPointIndex]>>= fValue;
                    sal_Int32 nNumberFormatKey = xDataSequence->getNumberFormatKeyByIndex( nPointIndex );
                    aY_First = aNumberFormatterWrapper.getFormattedString( nNumberFormatKey, fValue, nLabelColor, bColorChanged );
                }
                else if( aRole == "values-min" )
                {
                    aData[nPointIndex]>>= fValue;
                    sal_Int32 nNumberFormatKey = xDataSequence->getNumberFormatKeyByIndex( nPointIndex );
                    aY_Min = aNumberFormatterWrapper.getFormattedString( nNumberFormatKey, fValue, nLabelColor, bColorChanged );
                }
                else if( aRole == "values-max" )
                {
                    aData[nPointIndex]>>= fValue;
                    sal_Int32 nNumberFormatKey = xDataSequence->getNumberFormatKeyByIndex( nPointIndex );
                    aY_Max = aNumberFormatterWrapper.getFormattedString( nNumberFormatKey, fValue, nLabelColor, bColorChanged );
                }
                else if( aRole == "values-last" )
                {
                    aData[nPointIndex]>>= fValue;
                    sal_Int32 nNumberFormatKey = xDataSequence->getNumberFormatKeyByIndex( nPointIndex );
                    aY_Last = aNumberFormatterWrapper.getFormattedString( nNumberFormatKey, fValue, nLabelColor, bColorChanged );
                }
                else if( aRole == "values-size" )
                {
                    aData[nPointIndex]>>= fValue;
                    sal_Int32 nNumberFormatKey = xDataSequence->getNumberFormatKeyByIndex( nPointIndex );
                    a_Size = aNumberFormatterWrapper.getFormattedString( nNumberFormatKey, fValue, nLabelColor, bColorChanged );
                }
            }
        }
        catch (const lang::DisposedException&)
        {
            TOOLS_WARN_EXCEPTION( "chart2", "unexpected exception caught" );
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }

    if( aX.isEmpty() )
    {
        ChartModel& rModel = dynamic_cast<ChartModel&>(*xChartModel);
        aRet = ExplicitCategoriesProvider::getCategoryByIndex( xCooSys, rModel, nPointIndex );
    }
    else
    {
        aRet = aX;
    }

    OUString aSeparator( u" "_ustr );

    lcl_addText( aRet, aSeparator, aY );
    lcl_addText( aRet, aSeparator, aY_First );
    lcl_addText( aRet, aSeparator, aY_Min );
    lcl_addText( aRet, aSeparator, aY_Max );
    lcl_addText( aRet, aSeparator, aY_Last );
    lcl_addText( aRet, aSeparator, a_Size );

    return aRet;
}

} //end anonymous namespace

OUString ObjectNameProvider::getName( ObjectType eObjectType, bool bPlural )
{
    OUString aRet;
    switch( eObjectType )
    {
        case OBJECTTYPE_PAGE:
                aRet=SchResId(STR_OBJECT_PAGE);
                break;
        case OBJECTTYPE_TITLE:
            {
                if(bPlural)
                    aRet=SchResId(STR_OBJECT_TITLES);
                else
                    aRet=SchResId(STR_OBJECT_TITLE);
            }
                break;
        case OBJECTTYPE_LEGEND:
                aRet=SchResId(STR_OBJECT_LEGEND);
                break;
        case OBJECTTYPE_LEGEND_ENTRY:
                aRet=SchResId(STR_OBJECT_LEGEND_SYMBOL);//@todo change string if we do differentiate symbol and legend entry in future
                break;
        case OBJECTTYPE_DIAGRAM:
                aRet=SchResId(STR_OBJECT_DIAGRAM);
                break;
        case OBJECTTYPE_DIAGRAM_WALL:
                aRet=SchResId(STR_OBJECT_DIAGRAM_WALL);
                break;
        case OBJECTTYPE_DIAGRAM_FLOOR:
                aRet=SchResId(STR_OBJECT_DIAGRAM_FLOOR);
                break;
        case OBJECTTYPE_AXIS:
            {
                if(bPlural)
                    aRet=SchResId(STR_OBJECT_AXES);
                else
                    aRet=SchResId(STR_OBJECT_AXIS);
            }
                break;
        case OBJECTTYPE_AXIS_UNITLABEL:
                aRet=SchResId(STR_OBJECT_LABEL);//@todo maybe a more concrete name
                break;
        case OBJECTTYPE_GRID:
        case OBJECTTYPE_SUBGRID: //maybe todo: different names for subgrids
            {
                if(bPlural)
                    aRet=SchResId(STR_OBJECT_GRIDS);
                else
                    aRet=SchResId(STR_OBJECT_GRID);
            }
                break;
        case OBJECTTYPE_DATA_SERIES:
            {
                if(bPlural)
                    aRet=SchResId(STR_OBJECT_DATASERIES_PLURAL);
                else
                    aRet=SchResId(STR_OBJECT_DATASERIES);
            }
                break;
        case OBJECTTYPE_DATA_POINT:
            {
                if(bPlural)
                    aRet=SchResId(STR_OBJECT_DATAPOINTS);
                else
                    aRet=SchResId(STR_OBJECT_DATAPOINT);
            }
                break;
        case OBJECTTYPE_DATA_LABELS:
                aRet=SchResId(STR_OBJECT_DATALABELS);
                break;
        case OBJECTTYPE_DATA_LABEL:
                aRet=SchResId(STR_OBJECT_LABEL);
                break;
        case OBJECTTYPE_DATA_ERRORS_X:
                aRet=SchResId(STR_OBJECT_ERROR_BARS_X);
                break;
        case OBJECTTYPE_DATA_ERRORS_Y:
                aRet=SchResId(STR_OBJECT_ERROR_BARS_Y);
                break;
        case OBJECTTYPE_DATA_ERRORS_Z:
                aRet=SchResId(STR_OBJECT_ERROR_BARS_Z);
                break;
        case OBJECTTYPE_DATA_AVERAGE_LINE:
                aRet=SchResId(STR_OBJECT_AVERAGE_LINE);
                break;
        case OBJECTTYPE_DATA_CURVE:
            {
                if(bPlural)
                    aRet=SchResId(STR_OBJECT_CURVES);
                else
                    aRet=SchResId(STR_OBJECT_CURVE);
            }
                break;
        case OBJECTTYPE_DATA_STOCK_RANGE:
                break;
        case OBJECTTYPE_DATA_STOCK_LOSS:
                aRet=SchResId(STR_OBJECT_STOCK_LOSS);
                break;
        case OBJECTTYPE_DATA_STOCK_GAIN:
                aRet=SchResId(STR_OBJECT_STOCK_GAIN);
                break;
        case OBJECTTYPE_DATA_CURVE_EQUATION:
                aRet=SchResId(STR_OBJECT_CURVE_EQUATION);
                break;
        case OBJECTTYPE_DATA_TABLE:
                aRet=SchResId(STR_DATA_TABLE);
                break;
        default: //OBJECTTYPE_UNKNOWN
            ;
    }
    return aRet;
}

OUString ObjectNameProvider::getAxisName( std::u16string_view rObjectCID
                        , const rtl::Reference<::chart::ChartModel>& xChartModel  )
{
    OUString aRet;

    rtl::Reference< ::chart::Axis > xAxis =
        dynamic_cast<::chart::Axis*>(ObjectIdentifier::getObjectPropertySet( rObjectCID , xChartModel ).get());

    sal_Int32 nCooSysIndex = 0;
    sal_Int32 nDimensionIndex = 0;
    sal_Int32 nAxisIndex = 0;
    AxisHelper::getIndicesForAxis( xAxis, xChartModel->getFirstChartDiagram(), nCooSysIndex, nDimensionIndex, nAxisIndex );

    switch(nDimensionIndex)
    {
        case 0://x-axis
            if( nAxisIndex == 0 )
                aRet=SchResId(STR_OBJECT_AXIS_X);
            else
                aRet=SchResId(STR_OBJECT_SECONDARY_X_AXIS);
            break;
        case 1://y-axis
            if( nAxisIndex == 0 )
                aRet=SchResId(STR_OBJECT_AXIS_Y);
            else
                aRet=SchResId(STR_OBJECT_SECONDARY_Y_AXIS);
            break;
        case 2://z-axis
            aRet=SchResId(STR_OBJECT_AXIS_Z);
            break;
        default://axis
            aRet=SchResId(STR_OBJECT_AXIS);
            break;
    }

    return aRet;
}

OUString ObjectNameProvider::getTitleNameByType( TitleHelper::eTitleType eType )
{
    OUString aRet;

    switch(eType)
    {
        case TitleHelper::MAIN_TITLE:
            aRet=SchResId(STR_OBJECT_TITLE_MAIN);
            break;
        case TitleHelper::SUB_TITLE:
            aRet=SchResId(STR_OBJECT_TITLE_SUB);
            break;
        case TitleHelper::X_AXIS_TITLE:
            aRet=SchResId(STR_OBJECT_TITLE_X_AXIS);
            break;
        case TitleHelper::Y_AXIS_TITLE:
            aRet=SchResId(STR_OBJECT_TITLE_Y_AXIS);
            break;
        case TitleHelper::Z_AXIS_TITLE:
            aRet=SchResId(STR_OBJECT_TITLE_Z_AXIS);
            break;
        case TitleHelper::SECONDARY_X_AXIS_TITLE:
            aRet=SchResId(STR_OBJECT_TITLE_SECONDARY_X_AXIS);
            break;
        case TitleHelper::SECONDARY_Y_AXIS_TITLE:
            aRet=SchResId(STR_OBJECT_TITLE_SECONDARY_Y_AXIS);
            break;
        default:
            OSL_FAIL("unknown title type");
            break;
    }

    if( aRet.isEmpty() )
        aRet=SchResId(STR_OBJECT_TITLE);

    return aRet;
}

OUString ObjectNameProvider::getTitleName( std::u16string_view rObjectCID
                        , const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    OUString aRet;

    rtl::Reference<Title> xTitle =
        dynamic_cast<Title*>(ObjectIdentifier::getObjectPropertySet( rObjectCID , xChartModel ).get());
    if( xTitle )
    {
        TitleHelper::eTitleType eType;
        if( TitleHelper::getTitleType( eType, xTitle, xChartModel ) )
            aRet = ObjectNameProvider::getTitleNameByType( eType );
    }
    if( aRet.isEmpty() )
        aRet=SchResId(STR_OBJECT_TITLE);

    return aRet;
}

OUString ObjectNameProvider::getGridName( std::u16string_view rObjectCID
                        , const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    OUString aRet;

    sal_Int32 nCooSysIndex = -1;
    sal_Int32 nDimensionIndex = -1;
    sal_Int32 nAxisIndex = -1;
    rtl::Reference< Axis > xAxis = ObjectIdentifier::getAxisForCID( rObjectCID , xChartModel );
    AxisHelper::getIndicesForAxis( xAxis, xChartModel->getFirstChartDiagram()
              , nCooSysIndex , nDimensionIndex, nAxisIndex );

    bool bMainGrid = (ObjectIdentifier::getObjectType( rObjectCID ) == OBJECTTYPE_GRID);

    if( bMainGrid )
    {
        switch(nDimensionIndex)
        {
            case 0://x-axis
                aRet=SchResId(STR_OBJECT_GRID_MAJOR_X);
                break;
            case 1://y-axis
                aRet=SchResId(STR_OBJECT_GRID_MAJOR_Y);
                break;
            case 2://z-axis
                aRet=SchResId(STR_OBJECT_GRID_MAJOR_Z);
                break;
            default://axis
                aRet=SchResId(STR_OBJECT_GRID);
                break;
        }
    }
    else
    {
        switch(nDimensionIndex)
        {
            case 0://x-axis
                aRet=SchResId(STR_OBJECT_GRID_MINOR_X);
                break;
            case 1://y-axis
                aRet=SchResId(STR_OBJECT_GRID_MINOR_Y);
                break;
            case 2://z-axis
                aRet=SchResId(STR_OBJECT_GRID_MINOR_Z);
                break;
            default://axis
                aRet=SchResId(STR_OBJECT_GRID);
                break;
        }
    }
    return aRet;
}

OUString ObjectNameProvider::getHelpText( std::u16string_view rObjectCID, const rtl::Reference<::chart::ChartModel>& xChartModel, bool bVerbose )
{
    OUString aRet;
    ObjectType eObjectType( ObjectIdentifier::getObjectType(rObjectCID) );
    if( eObjectType == OBJECTTYPE_AXIS )
    {
        aRet=ObjectNameProvider::getAxisName( rObjectCID, xChartModel );
    }
    else if( eObjectType == OBJECTTYPE_GRID
        || eObjectType == OBJECTTYPE_SUBGRID )
    {
        aRet=ObjectNameProvider::getGridName( rObjectCID, xChartModel );
    }
    else if( eObjectType == OBJECTTYPE_TITLE )
    {
        aRet=ObjectNameProvider::getTitleName( rObjectCID, xChartModel );
    }
    else if( eObjectType == OBJECTTYPE_DATA_SERIES )
    {
        aRet = lcl_getFullSeriesName( rObjectCID, xChartModel );
    }
    else if( eObjectType == OBJECTTYPE_DATA_POINT )
    {
        if( bVerbose )
        {
            aRet= SchResId(STR_TIP_DATAPOINT_INDEX) + "\n"
                + SchResId(STR_TIP_DATASERIES) + "\n"
                + SchResId(STR_TIP_DATAPOINT_VALUES);
        }
        else
            aRet=SchResId(STR_TIP_DATAPOINT);

        rtl::Reference< Diagram > xDiagram( xChartModel->getFirstChartDiagram() );
        rtl::Reference< DataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( rObjectCID , xChartModel );
        if( xDiagram.is() && xSeries.is() )
        {
            sal_Int32 nPointIndex = o3tl::toInt32(ObjectIdentifier::getParticleID(rObjectCID));

            //replace data point index
            OUString aWildcard(  u"%POINTNUMBER"_ustr );
            sal_Int32 nIndex = aRet.indexOf( aWildcard );
            if( nIndex != -1 )
            {
                aRet = aRet.replaceAt( nIndex, aWildcard.getLength(), OUString::number(nPointIndex+1) );
            }

            //replace data series index
            aWildcard =  "%SERIESNUMBER";
            nIndex = aRet.indexOf( aWildcard );
            if( nIndex != -1 )
            {
                std::vector< rtl::Reference< DataSeries > > aSeriesVector =
                    xDiagram->getDataSeries();
                sal_Int32 nSeriesIndex = -1;
                for( nSeriesIndex=aSeriesVector.size();nSeriesIndex--;)
                {
                    if( aSeriesVector[nSeriesIndex] == xSeries )
                    {
                        break;
                    }
                }

                OUString aReplacement( OUString::number(nSeriesIndex+1) );
                aRet = aRet.replaceAt( nIndex, aWildcard.getLength(), aReplacement );
            }

            //replace point values
            aWildcard =  "%POINTVALUES";
            nIndex = aRet.indexOf( aWildcard );
            if( nIndex != -1 )
                aRet = aRet.replaceAt( nIndex, aWildcard.getLength(), lcl_getDataPointValueText(
                xSeries,nPointIndex, DataSeriesHelper::getCoordinateSystemOfSeries(xSeries, xDiagram), xChartModel ) );

            //replace series name
            aWildcard = "%SERIESNAME";
            nIndex = aRet.indexOf( aWildcard );
            if( nIndex != -1 )
                aRet = aRet.replaceAt( nIndex, aWildcard.getLength(), lcl_getDataSeriesName( rObjectCID, xChartModel ) );
        }
    }
    else if( eObjectType == OBJECTTYPE_DATA_CURVE )
    {
        if( bVerbose )
        {
            aRet = SchResId( STR_OBJECT_CURVE_WITH_PARAMETERS );
            rtl::Reference< DataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID( rObjectCID , xChartModel ));

            if( xSeries.is())
            {
                sal_Int32 nCurveIndex = ObjectIdentifier::getIndexFromParticleOrCID( rObjectCID );
                rtl::Reference< RegressionCurveModel > xCurve = RegressionCurveHelper::getRegressionCurveAtIndex(xSeries, nCurveIndex);
                if( xCurve.is())
                {
                    try
                    {
                        Reference< chart2::XRegressionCurveCalculator > xCalculator( xCurve->getCalculator(), uno::UNO_SET_THROW );
                        sal_Int32 aDegree = 2;
                        sal_Int32 aPeriod = 2;
                        sal_Int32 aMovingType = css::chart2::MovingAverageType::Prior;
                        bool bForceIntercept = false;
                        double aInterceptValue = 0.0;
                        OUString aXName (u"x"_ustr), aYName (u"f(x)"_ustr);
                        const LocaleDataWrapper& rLocaleDataWrapper = Application::GetSettings().GetLocaleDataWrapper();
                        const OUString& aNumDecimalSep = rLocaleDataWrapper.getNumDecimalSep();
                        sal_Unicode cDecSeparator = aNumDecimalSep[0];

                        xCurve->getPropertyValue( u"PolynomialDegree"_ustr) >>= aDegree;
                        xCurve->getPropertyValue( u"MovingAveragePeriod"_ustr) >>= aPeriod;
                        xCurve->getPropertyValue( u"MovingAverageType"_ustr) >>= aMovingType;
                        xCurve->getPropertyValue( u"ForceIntercept"_ustr) >>= bForceIntercept;
                        if (bForceIntercept)
                                xCurve->getPropertyValue( u"InterceptValue"_ustr) >>= aInterceptValue;
                        uno::Reference< beans::XPropertySet > xEqProp( xCurve->getEquationProperties());
                        if( xEqProp.is())
                        {
                            if ( !(xEqProp->getPropertyValue( u"XName"_ustr) >>= aXName) )
                                aXName = "x";
                            if ( !(xEqProp->getPropertyValue( u"YName"_ustr) >>= aYName) )
                                aYName = "f(x)";
                        }
                        xCalculator->setRegressionProperties(aDegree, bForceIntercept, aInterceptValue, aPeriod, aMovingType);
                        xCalculator->setXYNames ( aXName, aYName );
                        RegressionCurveHelper::initializeCurveCalculator( xCalculator, xSeries, xChartModel );

                        // change text for Moving Average
                        if ( RegressionCurveHelper::getRegressionType( xCurve ) == SvxChartRegress::MovingAverage )
                        {
                            aRet = xCalculator->getRepresentation();
                        }
                        else
                        {
                            // replace formula
                            OUString aWildcard = u"%FORMULA"_ustr;
                            sal_Int32 nIndex = aRet.indexOf( aWildcard );
                            if( nIndex != -1 )
                            {
                                OUString aFormula ( xCalculator->getRepresentation() );
                                if ( cDecSeparator != '.' )
                                {
                                    aFormula = aFormula.replace( '.', cDecSeparator );
                                }
                                aRet = aRet.replaceAt( nIndex, aWildcard.getLength(), aFormula );
                            }

                            // replace r^2
                            aWildcard = "%RSQUARED";
                            nIndex = aRet.indexOf( aWildcard );
                            if( nIndex != -1 )
                            {
                                double fR( xCalculator->getCorrelationCoefficient());
                                aRet = aRet.replaceAt(
                                    nIndex, aWildcard.getLength(),
                                    ::rtl::math::doubleToUString(
                                        fR*fR, rtl_math_StringFormat_G, 4, cDecSeparator, true ));
                            }
                        }
                    }
                    catch( const uno::Exception & )
                    {
                        DBG_UNHANDLED_EXCEPTION("chart2");
                    }
                }
            }
        }
        else
        {
            rtl::Reference< DataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID(rObjectCID , xChartModel));
            aRet += getName(eObjectType);

            if( xSeries.is())
            {
                sal_Int32 nCurveIndex = ObjectIdentifier::getIndexFromParticleOrCID( rObjectCID );
                rtl::Reference< RegressionCurveModel > xCurve( RegressionCurveHelper::getRegressionCurveAtIndex(xSeries, nCurveIndex) );
                if( xCurve.is())
                {
                    aRet += " (" + RegressionCurveHelper::getRegressionCurveName(xCurve) + " )";
                }
            }
        }
    }
    else if( eObjectType == OBJECTTYPE_DATA_AVERAGE_LINE )
    {
        if( bVerbose )
        {
            aRet = SchResId(STR_OBJECT_AVERAGE_LINE_WITH_PARAMETERS);
            rtl::Reference< DataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID( rObjectCID , xChartModel ));
            if( xSeries.is())
            {
                rtl::Reference< RegressionCurveModel > xCurve( RegressionCurveHelper::getMeanValueLine( xSeries ));
                if( xCurve.is())
                {
                    try
                    {
                        Reference< chart2::XRegressionCurveCalculator > xCalculator( xCurve->getCalculator(), uno::UNO_SET_THROW );
                        RegressionCurveHelper::initializeCurveCalculator( xCalculator, xSeries, xChartModel );

                        const LocaleDataWrapper& rLocaleDataWrapper = Application::GetSettings().GetLocaleDataWrapper();
                        const OUString& aNumDecimalSep = rLocaleDataWrapper.getNumDecimalSep();
                        sal_Unicode cDecSeparator = aNumDecimalSep[0];

                        OUString aWildcard( u"%AVERAGE_VALUE"_ustr );
                        sal_Int32 nIndex = aRet.indexOf( aWildcard );
                        // as the curve is constant, the value at any x-value is ok
                        if( nIndex != -1 )
                        {
                            const double fMeanValue( xCalculator->getCurveValue( 0.0 ));
                            aRet = aRet.replaceAt(
                                nIndex, aWildcard.getLength(),
                                ::rtl::math::doubleToUString(
                                    fMeanValue, rtl_math_StringFormat_G, 4, cDecSeparator, true ));
                        }

                        // replace standard deviation
                        aWildcard = "%STD_DEVIATION";
                        nIndex = aRet.indexOf( aWildcard );
                        if( nIndex != -1 )
                        {
                            const double fStdDev( xCalculator->getCorrelationCoefficient());
                            aRet = aRet.replaceAt(
                                nIndex, aWildcard.getLength(),
                                ::rtl::math::doubleToUString(
                                    fStdDev, rtl_math_StringFormat_G, 4, cDecSeparator, true ));
                        }
                    }
                    catch( const uno::Exception & )
                    {
                        DBG_UNHANDLED_EXCEPTION("chart2");
                    }
                }
            }
        }
        else
        {
            // non-verbose
            aRet = ObjectNameProvider::getName( eObjectType );
        }
    }
    else
    {
        aRet = ObjectNameProvider::getName( eObjectType );
    }
    return aRet;
}

OUString ObjectNameProvider::getSelectedObjectText( std::u16string_view rObjectCID, const rtl::Reference<::chart::ChartModel>& xChartDocument )
{
    OUString aRet;
    ObjectType eObjectType( ObjectIdentifier::getObjectType(rObjectCID) );

    if( eObjectType == OBJECTTYPE_DATA_POINT )
    {
        aRet = SchResId( STR_STATUS_DATAPOINT_MARKED );

        rtl::Reference< Diagram > xDiagram( xChartDocument->getFirstChartDiagram() );
        rtl::Reference< DataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( rObjectCID , xChartDocument );
        if( xDiagram.is() && xSeries.is() )
        {
            sal_Int32 nPointIndex = o3tl::toInt32( ObjectIdentifier::getParticleID(rObjectCID) );

            // replace data point index
            replaceParamterInString( aRet, u"%POINTNUMBER", OUString::number( nPointIndex + 1 ));

            // replace data series index
            {
                std::vector< rtl::Reference< DataSeries > > aSeriesVector(
                    xDiagram->getDataSeries() );
                sal_Int32 nSeriesIndex = -1;
                for( nSeriesIndex=aSeriesVector.size();nSeriesIndex--;)
                {
                    if( aSeriesVector[nSeriesIndex] == xSeries )
                        break;
                }
                replaceParamterInString( aRet, u"%SERIESNUMBER", OUString::number( nSeriesIndex + 1 ) );
            }

            // replace point value
            replaceParamterInString( aRet, u"%POINTVALUES", lcl_getDataPointValueText(
                xSeries, nPointIndex, DataSeriesHelper::getCoordinateSystemOfSeries(xSeries, xDiagram), xChartDocument ) );
        }
    }
    else
    {
        // use the verbose text including the formula for trend lines
        const bool bVerbose( eObjectType == OBJECTTYPE_DATA_CURVE || eObjectType == OBJECTTYPE_DATA_AVERAGE_LINE );
        const OUString aHelpText( getHelpText( rObjectCID, xChartDocument, bVerbose ));
        if( !aHelpText.isEmpty())
        {
            aRet = SchResId( STR_STATUS_OBJECT_MARKED );
            replaceParamterInString( aRet, u"%OBJECTNAME", aHelpText );
        }
    }

    return aRet;
}

OUString ObjectNameProvider::getNameForCID(
    std::u16string_view rObjectCID,
    const rtl::Reference<::chart::ChartModel>& xChartDocument )
{
    ObjectType eType( ObjectIdentifier::getObjectType( rObjectCID ));

    switch( eType )
    {
        case OBJECTTYPE_AXIS:
            return getAxisName( rObjectCID, xChartDocument );
        case OBJECTTYPE_TITLE:
            return getTitleName( rObjectCID, xChartDocument );
        case OBJECTTYPE_GRID:
        case OBJECTTYPE_SUBGRID:
            return getGridName( rObjectCID, xChartDocument );
        case OBJECTTYPE_DATA_SERIES:
            return lcl_getFullSeriesName( rObjectCID, xChartDocument );
        case OBJECTTYPE_DATA_POINT:
        case OBJECTTYPE_DATA_LABELS:
        case OBJECTTYPE_DATA_LABEL:
        case OBJECTTYPE_DATA_ERRORS_X:
        case OBJECTTYPE_DATA_ERRORS_Y:
        case OBJECTTYPE_DATA_ERRORS_Z:
        case OBJECTTYPE_DATA_AVERAGE_LINE:
        case OBJECTTYPE_DATA_CURVE:
        case OBJECTTYPE_DATA_CURVE_EQUATION:
            {
                OUString aRet = lcl_getFullSeriesName( rObjectCID, xChartDocument ) + " ";
                if( eType == OBJECTTYPE_DATA_POINT || eType == OBJECTTYPE_DATA_LABEL )
                {
                    aRet += getName( OBJECTTYPE_DATA_POINT  );
                    sal_Int32 nPointIndex = ObjectIdentifier::getIndexFromParticleOrCID( rObjectCID );
                    aRet += " " + OUString::number(nPointIndex+1);
                    if( eType == OBJECTTYPE_DATA_LABEL )
                    {
                        aRet += " " + getName( OBJECTTYPE_DATA_LABEL  );
                    }
                }
                else if (eType == OBJECTTYPE_DATA_CURVE || eType == OBJECTTYPE_DATA_CURVE_EQUATION)
                {
                    rtl::Reference< DataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID( rObjectCID , xChartDocument ));

                    aRet += " " + getName(eType);

                    if( xSeries.is())
                    {
                        sal_Int32 nCurveIndex = ObjectIdentifier::getIndexFromParticleOrCID( rObjectCID );
                        rtl::Reference< RegressionCurveModel > xCurve( RegressionCurveHelper::getRegressionCurveAtIndex(xSeries, nCurveIndex) );
                        if( xCurve.is())
                        {
                           aRet += " (" + RegressionCurveHelper::getRegressionCurveName(xCurve) + ")";
                        }
                    }
                }
                else
                {
                    aRet += getName( eType );
                }
                return aRet;
            }
        default:
            break;
    }

    return getName( eType );
}

OUString ObjectNameProvider::getName_ObjectForSeries(
        ObjectType eObjectType,
        std::u16string_view rSeriesCID,
        const rtl::Reference<::chart::ChartModel>& xChartDocument )
{
    rtl::Reference< DataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( rSeriesCID , xChartDocument );
    if( xSeries.is() )
    {
        OUString aRet = SchResId(STR_OBJECT_FOR_SERIES);
        replaceParamterInString( aRet, u"%OBJECTNAME", getName( eObjectType ) );
        replaceParamterInString( aRet, u"%SERIESNAME", lcl_getDataSeriesName( rSeriesCID, xChartDocument ) );
        return aRet;
    }
    else
        return ObjectNameProvider::getName_ObjectForAllSeries( eObjectType );
}

OUString ObjectNameProvider::getName_ObjectForAllSeries( ObjectType eObjectType )
{
    OUString aRet = SchResId(STR_OBJECT_FOR_ALL_SERIES);
    replaceParamterInString( aRet, u"%OBJECTNAME", getName( eObjectType, true /*bPlural*/ ) );
    return aRet;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
