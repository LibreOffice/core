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


#include "ObjectNameProvider.hxx"
#include "ResId.hxx"
#include "Strings.hrc"
#include "macros.hxx"
#include "AxisHelper.hxx"
#include "ChartModelHelper.hxx"
#include "DiagramHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "TitleHelper.hxx"
#include "AxisIndexDefines.hxx"
#include "ExplicitCategoriesProvider.hxx"
#include "CommonConverters.hxx"
#include "NumberFormatterWrapper.hxx"
#include "RegressionCurveHelper.hxx"
#include <rtl/math.hxx>
#include <tools/string.hxx>

#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

namespace
{

OUString lcl_getDataSeriesName( const OUString& rObjectCID, const Reference< frame::XModel >& xChartModel )
{
    OUString aRet;

    Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    Reference< XDataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID( rObjectCID , xChartModel ), uno::UNO_QUERY );
    if( xDiagram.is() && xSeries.is() )
    {
        Reference< XChartType > xChartType( DiagramHelper::getChartTypeOfSeries( xDiagram, xSeries ) );
        if( xChartType.is() )
        {
            aRet = ::chart::DataSeriesHelper::getDataSeriesLabel(
                    xSeries, xChartType->getRoleOfSequenceForSeriesLabel() ) ;
        }
    }

    return aRet;
}

OUString lcl_getFullSeriesName( const OUString& rObjectCID, const Reference< frame::XModel >& xChartModel )
{
    OUString aRet = String(SchResId(STR_TIP_DATASERIES));
    OUString aWildcard( "%SERIESNAME" );
    sal_Int32 nIndex = aRet.indexOf( aWildcard );
    if( nIndex != -1 )
        aRet = aRet.replaceAt( nIndex, aWildcard.getLength(), lcl_getDataSeriesName( rObjectCID, xChartModel ) );
    return aRet;
}

void lcl_addText( OUString& rOut, const OUString& rSeparator, const OUString& rNext )
{
    if( !(rOut.isEmpty() || rNext.isEmpty()) )
        rOut+=rSeparator;
    if( !rNext.isEmpty() )
        rOut+=rNext;
}

OUString lcl_getDataPointValueText( const Reference< XDataSeries >& xSeries, sal_Int32 nPointIndex,
                                    const Reference< XCoordinateSystem >& xCooSys,
                                    const Reference< frame::XModel >& xChartModel )
{

    OUString aRet;

    Reference<data::XDataSource> xDataSource(
            uno::Reference<data::XDataSource>( xSeries, uno::UNO_QUERY ) );
    if(!xDataSource.is())
        return aRet;

    Sequence< Reference< data::XLabeledDataSequence > > aDataSequences( xDataSource->getDataSequences() );

    OUString aX, aY, aY_Min, aY_Max, aY_First, aY_Last, a_Size;
    double fValue = 0;

    uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( xChartModel, uno::UNO_QUERY );
    NumberFormatterWrapper aNumberFormatterWrapper( xNumberFormatsSupplier );
    sal_Int32 nLabelColor = 0;//dummy
    bool bColorChanged;//dummy

    for(sal_Int32 nN = aDataSequences.getLength();nN--;)
    {
        uno::Reference<data::XDataSequence>  xDataSequence( aDataSequences[nN]->getValues());
        if( !xDataSequence.is() )
            continue;
        Sequence< Any > aData( xDataSequence->getData() );
        if( nPointIndex >= aData.getLength() )
            continue;
        uno::Reference<beans::XPropertySet> xProp(xDataSequence, uno::UNO_QUERY );
        if( xProp.is())
        {
            try
            {
                uno::Any aARole = xProp->getPropertyValue( "Role" );
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
            catch( const uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }
    }

    if( aX.isEmpty() )
    {
        aRet = ExplicitCategoriesProvider::getCategoryByIndex( xCooSys, xChartModel, nPointIndex );
    }
    else
    {
        aRet = aX;
    }

    OUString aSeparator( " " );

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
                aRet=String(SchResId(STR_OBJECT_PAGE));
                break;
        case OBJECTTYPE_TITLE:
            {
                if(bPlural)
                    aRet=String(SchResId(STR_OBJECT_TITLES));
                else
                    aRet=String(SchResId(STR_OBJECT_TITLE));
            }
                break;
        case OBJECTTYPE_LEGEND:
                aRet=String(SchResId(STR_OBJECT_LEGEND));
                break;
        case OBJECTTYPE_LEGEND_ENTRY:
                aRet=String(SchResId(STR_OBJECT_LEGEND_SYMBOL));//@todo change string if we do differenciate symbol and legend entry in future
                break;
        case OBJECTTYPE_DIAGRAM:
                aRet=String(SchResId(STR_OBJECT_DIAGRAM));
                break;
        case OBJECTTYPE_DIAGRAM_WALL:
                aRet=String(SchResId(STR_OBJECT_DIAGRAM_WALL));
                break;
        case OBJECTTYPE_DIAGRAM_FLOOR:
                aRet=String(SchResId(STR_OBJECT_DIAGRAM_FLOOR));
                break;
        case OBJECTTYPE_AXIS:
            {
                if(bPlural)
                    aRet=String(SchResId(STR_OBJECT_AXES));
                else
                    aRet=String(SchResId(STR_OBJECT_AXIS));
            }
                break;
        case OBJECTTYPE_AXIS_UNITLABEL:
                aRet=String(SchResId(STR_OBJECT_LABEL));//@todo maybe a more concrete name
                break;
        case OBJECTTYPE_GRID:
        case OBJECTTYPE_SUBGRID: //maybe todo: different names for subgrids
            {
                if(bPlural)
                    aRet=String(SchResId(STR_OBJECT_GRIDS));
                else
                    aRet=String(SchResId(STR_OBJECT_GRID));
            }
                break;
        case OBJECTTYPE_DATA_SERIES:
            {
                if(bPlural)
                    aRet=String(SchResId(STR_OBJECT_DATASERIES_PLURAL));
                else
                    aRet=String(SchResId(STR_OBJECT_DATASERIES));
            }
                break;
        case OBJECTTYPE_DATA_POINT:
            {
                if(bPlural)
                    aRet=String(SchResId(STR_OBJECT_DATAPOINTS));
                else
                    aRet=String(SchResId(STR_OBJECT_DATAPOINT));
            }
                break;
        case OBJECTTYPE_DATA_LABELS:
                aRet=String(SchResId(STR_OBJECT_DATALABELS));
                break;
        case OBJECTTYPE_DATA_LABEL:
                aRet=String(SchResId(STR_OBJECT_LABEL));
                break;
        case OBJECTTYPE_DATA_ERRORS_X:
                aRet=String(SchResId(STR_OBJECT_ERROR_BARS_X));
                break;
        case OBJECTTYPE_DATA_ERRORS_Y:
                aRet=String(SchResId(STR_OBJECT_ERROR_BARS_Y));
                break;
        case OBJECTTYPE_DATA_ERRORS_Z:
                aRet=String(SchResId(STR_OBJECT_ERROR_BARS_Z));
                break;
        case OBJECTTYPE_DATA_AVERAGE_LINE:
                aRet=String(SchResId(STR_OBJECT_AVERAGE_LINE));
                break;
        case OBJECTTYPE_DATA_CURVE:
            {
                if(bPlural)
                    aRet=String(SchResId(STR_OBJECT_CURVES));
                else
                    aRet=String(SchResId(STR_OBJECT_CURVE));
            }
                break;
        case OBJECTTYPE_DATA_STOCK_RANGE:
                break;
        case OBJECTTYPE_DATA_STOCK_LOSS:
                aRet=String(SchResId(STR_OBJECT_STOCK_LOSS));
                break;
        case OBJECTTYPE_DATA_STOCK_GAIN:
                aRet=String(SchResId(STR_OBJECT_STOCK_GAIN));
                break;
        case OBJECTTYPE_DATA_CURVE_EQUATION:
                aRet=String(SchResId(STR_OBJECT_CURVE_EQUATION));
                break;
        default: //OBJECTTYPE_UNKNOWN
            ;
    }
    return aRet;
}

OUString ObjectNameProvider::getAxisName( const OUString& rObjectCID
                        , const uno::Reference< frame::XModel >& xChartModel  )
{
    OUString aRet;



    Reference< XAxis > xAxis(
        ObjectIdentifier::getObjectPropertySet( rObjectCID , xChartModel ), uno::UNO_QUERY );

    sal_Int32 nCooSysIndex = 0;
    sal_Int32 nDimensionIndex = 0;
    sal_Int32 nAxisIndex = 0;
    AxisHelper::getIndicesForAxis( xAxis, ChartModelHelper::findDiagram( xChartModel ), nCooSysIndex, nDimensionIndex, nAxisIndex );

    switch(nDimensionIndex)
    {
        case 0://x-axis
            if( nAxisIndex == 0 )
                aRet=String(SchResId(STR_OBJECT_AXIS_X));
            else
                aRet=String(SchResId(STR_OBJECT_SECONDARY_X_AXIS));
            break;
        case 1://y-axis
            if( nAxisIndex == 0 )
                aRet=String(SchResId(STR_OBJECT_AXIS_Y));
            else
                aRet=String(SchResId(STR_OBJECT_SECONDARY_Y_AXIS));
            break;
        case 2://z-axis
            aRet=String(SchResId(STR_OBJECT_AXIS_Z));
            break;
        default://axis
            aRet=String(SchResId(STR_OBJECT_AXIS));
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
            aRet=String(SchResId(STR_OBJECT_TITLE_MAIN));
            break;
        case TitleHelper::SUB_TITLE:
            aRet=String(SchResId(STR_OBJECT_TITLE_SUB));
            break;
        case TitleHelper::X_AXIS_TITLE:
            aRet=String(SchResId(STR_OBJECT_TITLE_X_AXIS));
            break;
        case TitleHelper::Y_AXIS_TITLE:
            aRet=String(SchResId(STR_OBJECT_TITLE_Y_AXIS));
            break;
        case TitleHelper::Z_AXIS_TITLE:
            aRet=String(SchResId(STR_OBJECT_TITLE_Z_AXIS));
            break;
        case TitleHelper::SECONDARY_X_AXIS_TITLE:
            aRet=String(SchResId(STR_OBJECT_TITLE_SECONDARY_X_AXIS));
            break;
        case TitleHelper::SECONDARY_Y_AXIS_TITLE:
            aRet=String(SchResId(STR_OBJECT_TITLE_SECONDARY_Y_AXIS));
            break;
        default:
            OSL_FAIL("unknown title type");
            break;
    }

    if( aRet.isEmpty() )
        aRet=String(SchResId(STR_OBJECT_TITLE));

    return aRet;
}

OUString ObjectNameProvider::getTitleName( const OUString& rObjectCID
                        , const Reference< frame::XModel >& xChartModel )
{
    OUString aRet;

    Reference< XTitle > xTitle(
        ObjectIdentifier::getObjectPropertySet( rObjectCID , xChartModel ), uno::UNO_QUERY );
    if( xTitle.is() )
    {
        TitleHelper::eTitleType eType;
        if( TitleHelper::getTitleType( eType, xTitle, xChartModel ) )
            aRet = ObjectNameProvider::getTitleNameByType( eType );
    }
    if( aRet.isEmpty() )
        aRet=String(SchResId(STR_OBJECT_TITLE));

    return aRet;
}

OUString ObjectNameProvider::getGridName( const OUString& rObjectCID
                        , const uno::Reference< frame::XModel >& xChartModel )
{
    OUString aRet;


    sal_Int32 nCooSysIndex = -1;
    sal_Int32 nDimensionIndex = -1;
    sal_Int32 nAxisIndex = -1;
    Reference< XAxis > xAxis( ObjectIdentifier::getAxisForCID( rObjectCID , xChartModel ) );
    AxisHelper::getIndicesForAxis( xAxis, ChartModelHelper::findDiagram( xChartModel )
              , nCooSysIndex , nDimensionIndex, nAxisIndex );

    bool bMainGrid = (ObjectIdentifier::getObjectType( rObjectCID ) == OBJECTTYPE_GRID);

    if( bMainGrid )
    {
        switch(nDimensionIndex)
        {
            case 0://x-axis
                aRet=String(SchResId(STR_OBJECT_GRID_MAJOR_X));
                break;
            case 1://y-axis
                aRet=String(SchResId(STR_OBJECT_GRID_MAJOR_Y));
                break;
            case 2://z-axis
                aRet=String(SchResId(STR_OBJECT_GRID_MAJOR_Z));
                break;
            default://axis
                aRet=String(SchResId(STR_OBJECT_GRID));
                break;
        }
    }
    else
    {
        switch(nDimensionIndex)
        {
            case 0://x-axis
                aRet=String(SchResId(STR_OBJECT_GRID_MINOR_X));
                break;
            case 1://y-axis
                aRet=String(SchResId(STR_OBJECT_GRID_MINOR_Y));
                break;
            case 2://z-axis
                aRet=String(SchResId(STR_OBJECT_GRID_MINOR_Z));
                break;
            default://axis
                aRet=String(SchResId(STR_OBJECT_GRID));
                break;
        }
    }
    return aRet;
}

OUString ObjectNameProvider::getHelpText( const OUString& rObjectCID, const Reference< chart2::XChartDocument >& xChartDocument, bool bVerbose )
{
    return getHelpText( rObjectCID, Reference< frame::XModel >( xChartDocument, uno::UNO_QUERY ), bVerbose );
}

OUString ObjectNameProvider::getHelpText( const OUString& rObjectCID, const Reference< frame::XModel >& xChartModel, bool bVerbose )
{
    OUString aRet;
    ObjectType eObjectType( ObjectIdentifier::getObjectType(rObjectCID) );
    if( OBJECTTYPE_AXIS == eObjectType )
    {
        aRet=ObjectNameProvider::getAxisName( rObjectCID, xChartModel );
    }
    else if( OBJECTTYPE_GRID == eObjectType
        || OBJECTTYPE_SUBGRID == eObjectType )
    {
        aRet=ObjectNameProvider::getGridName( rObjectCID, xChartModel );
    }
    else if( OBJECTTYPE_TITLE == eObjectType )
    {
        aRet=ObjectNameProvider::getTitleName( rObjectCID, xChartModel );
    }
    else if( OBJECTTYPE_DATA_SERIES == eObjectType )
    {
        aRet = lcl_getFullSeriesName( rObjectCID, xChartModel );
    }
    else if( OBJECTTYPE_DATA_POINT == eObjectType )
    {
        if( bVerbose )
        {
            OUString aNewLine( "\n" );

            aRet=String(SchResId(STR_TIP_DATAPOINT_INDEX));
            aRet+=aNewLine;
            aRet+=String(SchResId(STR_TIP_DATASERIES));
            aRet+=aNewLine;
            aRet+=String(SchResId(STR_TIP_DATAPOINT_VALUES));
        }
        else
            aRet=String(SchResId(STR_TIP_DATAPOINT));

        Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
        Reference< XDataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID( rObjectCID , xChartModel ), uno::UNO_QUERY );
        if( xDiagram.is() && xSeries.is() )
        {
            sal_Int32 nPointIndex( ObjectIdentifier::getParticleID(rObjectCID).toInt32() );

            //replace data point index
            sal_Int32 nIndex = -1;
            OUString aWildcard(  "%POINTNUMBER" );
            nIndex = aRet.indexOf( aWildcard );
            if( nIndex != -1 )
            {
                aRet = aRet.replaceAt( nIndex, aWildcard.getLength(), OUString::valueOf(nPointIndex+1) );
            }

            //replace data series index
            aWildcard =  "%SERIESNUMBER";
            nIndex = aRet.indexOf( aWildcard );
            if( nIndex != -1 )
            {
                ::std::vector< Reference< chart2::XDataSeries > > aSeriesVector(
                    DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );
                sal_Int32 nSeriesIndex = -1;
                for( nSeriesIndex=aSeriesVector.size();nSeriesIndex--;)
                {
                    if( aSeriesVector[nSeriesIndex] == xSeries )
                    {
                        break;
                    }
                }

                OUString aReplacement( OUString::valueOf(nSeriesIndex+1) );
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
    else if( OBJECTTYPE_DATA_CURVE == eObjectType )
    {
        if( bVerbose )
        {
            aRet = String( SchResId( STR_OBJECT_CURVE_WITH_PARAMETERS ));
            Reference< chart2::XDataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID( rObjectCID , xChartModel ));
            Reference< chart2::XRegressionCurveContainer > xCurveCnt( xSeries, uno::UNO_QUERY );
            if( xCurveCnt.is())
            {
                Reference< chart2::XRegressionCurve > xCurve( RegressionCurveHelper::getFirstCurveNotMeanValueLine( xCurveCnt ));
                if( xCurve.is())
                {
                    try
                    {
                        Reference< chart2::XRegressionCurveCalculator > xCalculator( xCurve->getCalculator(), uno::UNO_QUERY_THROW );
                        RegressionCurveHelper::initializeCurveCalculator( xCalculator, xSeries, xChartModel );

                        // replace formula
                        sal_Int32 nIndex = -1;
                        OUString aWildcard( "%FORMULA" );
                        nIndex = aRet.indexOf( aWildcard );
                        if( nIndex != -1 )
                            aRet = aRet.replaceAt( nIndex, aWildcard.getLength(), xCalculator->getRepresentation());

                        // replace r^2
                        aWildcard = "%RSQUARED";
                        nIndex = aRet.indexOf( aWildcard );
                        if( nIndex != -1 )
                        {
                            sal_Unicode aDecimalSep( '.' );
                            double fR( xCalculator->getCorrelationCoefficient());
                            aRet = aRet.replaceAt(
                                nIndex, aWildcard.getLength(),
                                ::rtl::math::doubleToUString(
                                    fR*fR, rtl_math_StringFormat_G, 4, aDecimalSep, true ));
                        }
                    }
                    catch( const uno::Exception & ex )
                    {
                        ASSERT_EXCEPTION( ex );
                    }
                }
            }
        }
        else
        {
            // non-verbose
            aRet = ObjectNameProvider::getName( eObjectType, false );
        }
    }
    else if( OBJECTTYPE_DATA_AVERAGE_LINE == eObjectType )
    {
        if( bVerbose )
        {
            aRet = String( SchResId( STR_OBJECT_AVERAGE_LINE_WITH_PARAMETERS ));
            Reference< chart2::XDataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID( rObjectCID , xChartModel ));
            Reference< chart2::XRegressionCurveContainer > xCurveCnt( xSeries, uno::UNO_QUERY );
            if( xCurveCnt.is())
            {
                Reference< chart2::XRegressionCurve > xCurve( RegressionCurveHelper::getMeanValueLine( xCurveCnt ));
                if( xCurve.is())
                {
                    try
                    {
                        Reference< chart2::XRegressionCurveCalculator > xCalculator( xCurve->getCalculator(), uno::UNO_QUERY_THROW );
                        RegressionCurveHelper::initializeCurveCalculator( xCalculator, xSeries, xChartModel );

                        sal_Unicode aDecimalSep( '.' );

                        sal_Int32 nIndex = -1;
                        OUString aWildcard( "%AVERAGE_VALUE" );
                        nIndex = aRet.indexOf( aWildcard );
                        // as the curve is constant, the value at any x-value is ok
                        if( nIndex != -1 )
                        {
                            const double fMeanValue( xCalculator->getCurveValue( 0.0 ));
                            aRet = aRet.replaceAt(
                                nIndex, aWildcard.getLength(),
                                ::rtl::math::doubleToUString(
                                    fMeanValue, rtl_math_StringFormat_G, 4, aDecimalSep, true ));
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
                                    fStdDev, rtl_math_StringFormat_G, 4, aDecimalSep, true ));
                        }
                    }
                    catch( const uno::Exception & ex )
                    {
                        ASSERT_EXCEPTION( ex );
                    }
                }
            }
        }
        else
        {
            // non-verbose
            aRet = ObjectNameProvider::getName( eObjectType, false );
        }
    }
    else
    {
        aRet = ObjectNameProvider::getName( eObjectType, false );
    }
    return aRet;
}

OUString ObjectNameProvider::getSelectedObjectText( const OUString & rObjectCID, const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xChartDocument )
{
    OUString aRet;
    ObjectType eObjectType( ObjectIdentifier::getObjectType(rObjectCID) );
    Reference< frame::XModel > xChartModel( xChartDocument, uno::UNO_QUERY );

    if( OBJECTTYPE_DATA_POINT == eObjectType )
    {
        aRet = String( SchResId( STR_STATUS_DATAPOINT_MARKED ));

        Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
        Reference< XDataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID( rObjectCID , xChartModel ), uno::UNO_QUERY );
        if( xDiagram.is() && xSeries.is() )
        {
            sal_Int32 nPointIndex( ObjectIdentifier::getParticleID(rObjectCID).toInt32() );

            // replace data point index
            replaceParamterInString( aRet, "%POINTNUMBER", OUString::valueOf( nPointIndex + 1 ));

            // replace data series index
            {
                ::std::vector< Reference< chart2::XDataSeries > > aSeriesVector(
                    DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );
                sal_Int32 nSeriesIndex = -1;
                for( nSeriesIndex=aSeriesVector.size();nSeriesIndex--;)
                {
                    if( aSeriesVector[nSeriesIndex] == xSeries )
                        break;
                }
                replaceParamterInString( aRet, "%SERIESNUMBER", OUString::valueOf( nSeriesIndex + 1 ) );
            }

            // replace point value
            replaceParamterInString( aRet, "%POINTVALUES", lcl_getDataPointValueText(
                xSeries, nPointIndex, DataSeriesHelper::getCoordinateSystemOfSeries(xSeries, xDiagram), xChartModel ) );
        }
    }
    else
    {
        // use the verbose text including the formula for trend lines
        const bool bVerbose( OBJECTTYPE_DATA_CURVE == eObjectType || OBJECTTYPE_DATA_AVERAGE_LINE == eObjectType );
        const OUString aHelpText( getHelpText( rObjectCID, xChartModel, bVerbose ));
        if( !aHelpText.isEmpty())
        {
            aRet = String( SchResId( STR_STATUS_OBJECT_MARKED ));
            replaceParamterInString( aRet, "%OBJECTNAME", aHelpText );
        }
    }

    return aRet;
}


OUString ObjectNameProvider::getNameForCID(
    const OUString& rObjectCID,
    const uno::Reference< chart2::XChartDocument >& xChartDocument )
{
    ObjectType eType( ObjectIdentifier::getObjectType( rObjectCID ));
    Reference< frame::XModel > xModel( xChartDocument, uno::UNO_QUERY );

    switch( eType )
    {
        case OBJECTTYPE_AXIS:
            return getAxisName( rObjectCID, xModel );
        case OBJECTTYPE_TITLE:
            return getTitleName( rObjectCID, xModel );
        case OBJECTTYPE_GRID:
        case OBJECTTYPE_SUBGRID:
            return getGridName( rObjectCID, xModel );
        case OBJECTTYPE_DATA_SERIES:
            return lcl_getFullSeriesName( rObjectCID, xModel );
        case OBJECTTYPE_DATA_POINT:
        case OBJECTTYPE_DATA_LABELS:
        case OBJECTTYPE_DATA_LABEL:
        case OBJECTTYPE_DATA_ERRORS_X:
        case OBJECTTYPE_DATA_ERRORS_Y:
        case OBJECTTYPE_DATA_ERRORS_Z:
        case OBJECTTYPE_DATA_CURVE:
        case OBJECTTYPE_DATA_AVERAGE_LINE:
        case OBJECTTYPE_DATA_CURVE_EQUATION:
            {
                OUString aRet = lcl_getFullSeriesName( rObjectCID, xModel );
                aRet += " ";
                if( eType == OBJECTTYPE_DATA_POINT || eType == OBJECTTYPE_DATA_LABEL )
                {
                    aRet += getName( OBJECTTYPE_DATA_POINT  );
                    sal_Int32 nPointIndex = ObjectIdentifier::getIndexFromParticleOrCID( rObjectCID );
                    aRet += " ";
                    aRet += OUString::valueOf(nPointIndex+1);

                    if( eType == OBJECTTYPE_DATA_LABEL )
                    {
                        aRet += " ";
                        aRet += getName( OBJECTTYPE_DATA_LABEL  );
                    }
                }
                else
                    aRet += getName( eType );
                return aRet;
            }
        default:
            break;
    }

    return getName( eType );
}

OUString ObjectNameProvider::getName_ObjectForSeries(
        ObjectType eObjectType,
        const OUString& rSeriesCID,
        const uno::Reference< chart2::XChartDocument >& xChartDocument )
{
    uno::Reference< frame::XModel> xChartModel( xChartDocument, uno::UNO_QUERY );
    Reference< XDataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID( rSeriesCID , xChartModel ), uno::UNO_QUERY );
    if( xSeries.is() )
    {
        OUString aRet = String(SchResId(STR_OBJECT_FOR_SERIES));
        replaceParamterInString( aRet, "%OBJECTNAME", getName( eObjectType, false /*bPlural*/ ) );
        replaceParamterInString( aRet, "%SERIESNAME", lcl_getDataSeriesName( rSeriesCID, xChartModel ) );
        return aRet;
    }
    else
        return ObjectNameProvider::getName_ObjectForAllSeries( eObjectType );
}

OUString ObjectNameProvider::getName_ObjectForAllSeries( ObjectType eObjectType )
{
    OUString aRet = String(SchResId(STR_OBJECT_FOR_ALL_SERIES));
    replaceParamterInString( aRet, "%OBJECTNAME", getName( eObjectType, true /*bPlural*/ ) );
    return aRet;
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
