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

#include "ChartController.hxx"
#include "ChartWindow.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "ObjectIdentifier.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "macros.hxx"
#include "dlg_ObjectProperties.hxx"
#include "dlg_View3D.hxx"
#include "dlg_InsertErrorBars.hxx"
#include "ViewElementListProvider.hxx"
#include "DataPointItemConverter.hxx"
#include "AxisItemConverter.hxx"
#include "MultipleChartConverters.hxx"
#include "TitleItemConverter.hxx"
#include "LegendItemConverter.hxx"
#include "RegressionCurveItemConverter.hxx"
#include "RegressionEquationItemConverter.hxx"
#include "ErrorBarItemConverter.hxx"
#include "ChartModelHelper.hxx"
#include "AxisHelper.hxx"
#include "TitleHelper.hxx"
#include "LegendHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "ColorPerPointHelper.hxx"
#include "DiagramHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "ControllerLockGuard.hxx"
#include "UndoGuard.hxx"
#include "ObjectNameProvider.hxx"
#include "ResId.hxx"
#include "Strings.hrc"
#include "ReferenceSizeProvider.hxx"
#include "RegressionCurveHelper.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>

//for auto_ptr
#include <memory>

// header for define RET_OK
#include <vcl/msgbox.hxx>
// for SolarMutex
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <svx/ActionDescriptionProvider.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

namespace
{

SAL_WNODEPRECATED_DECLARATIONS_PUSH
::comphelper::ItemConverter* createItemConverter(
    const OUString & aObjectCID
    , const uno::Reference< frame::XModel > & xChartModel
    , const uno::Reference< uno::XComponentContext > & xContext
    , SdrModel & rDrawModel
    , ExplicitValueProvider * pExplicitValueProvider = NULL
    , ::std::auto_ptr< ReferenceSizeProvider > pRefSizeProvider =
          ::std::auto_ptr< ReferenceSizeProvider >()
    )
{
    ::comphelper::ItemConverter* pItemConverter=NULL;

    //get type of selected object
    ObjectType eObjectType = ObjectIdentifier::getObjectType( aObjectCID );
    if( OBJECTTYPE_UNKNOWN==eObjectType )
    {
        OSL_FAIL("unknown ObjectType");
        return NULL;
    }

    OUString aParticleID = ObjectIdentifier::getParticleID( aObjectCID );
    bool bAffectsMultipleObjects = aParticleID == "ALLELEMENTS";
    if( !bAffectsMultipleObjects )
    {
        uno::Reference< beans::XPropertySet > xObjectProperties =
            ObjectIdentifier::getObjectPropertySet( aObjectCID, xChartModel );
        if(!xObjectProperties.is())
            return NULL;
        //create itemconverter for a single object
        switch(eObjectType)
        {
            case OBJECTTYPE_PAGE:
                pItemConverter =  new wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(),
                                        rDrawModel, uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                        wrapper::GraphicPropertyItemConverter::LINE_AND_FILL_PROPERTIES );
                    break;
            case OBJECTTYPE_TITLE:
            {
                ::std::auto_ptr< awt::Size > pRefSize;
                if( pRefSizeProvider.get() )
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getPageSize()));

                pItemConverter = new wrapper::TitleItemConverter( xObjectProperties,
                                                                  rDrawModel.GetItemPool(), rDrawModel,
                                                                  uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                                                  pRefSize );
            }
            break;
            case OBJECTTYPE_LEGEND:
            {
                ::std::auto_ptr< awt::Size > pRefSize;
                if( pRefSizeProvider.get() )
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getPageSize()));

                pItemConverter = new wrapper::LegendItemConverter( xObjectProperties,
                                                                   rDrawModel.GetItemPool(), rDrawModel,
                                                                   uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                                                   pRefSize );
            }
            break;
            case OBJECTTYPE_LEGEND_ENTRY:
                    break;
            case OBJECTTYPE_DIAGRAM:
                    break;
            case OBJECTTYPE_DIAGRAM_WALL:
            case OBJECTTYPE_DIAGRAM_FLOOR:
                pItemConverter =  new wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(),
                                        rDrawModel, uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                        wrapper::GraphicPropertyItemConverter::LINE_AND_FILL_PROPERTIES );
                    break;
            case OBJECTTYPE_AXIS:
            {
                ::std::auto_ptr< awt::Size > pRefSize;
                if( pRefSizeProvider.get() )
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getPageSize()));

                uno::Reference< beans::XPropertySet > xDiaProp;
                xDiaProp.set( ChartModelHelper::findDiagram( xChartModel ), uno::UNO_QUERY );

                // the second property set contains the property CoordinateOrigin
                // nOriginIndex is the index of the corresponding index of the
                // origin (x=0, y=1, z=2)

                ExplicitScaleData aExplicitScale;
                ExplicitIncrementData aExplicitIncrement;
                if( pExplicitValueProvider )
                    pExplicitValueProvider->getExplicitValuesForAxis(
                        uno::Reference< XAxis >( xObjectProperties, uno::UNO_QUERY ),
                        aExplicitScale, aExplicitIncrement );

                pItemConverter =  new wrapper::AxisItemConverter(
                    xObjectProperties, rDrawModel.GetItemPool(),
                    rDrawModel,
                    uno::Reference< chart2::XChartDocument >( xChartModel, uno::UNO_QUERY ),
                    &aExplicitScale, &aExplicitIncrement,
                    pRefSize );
            }
            break;
            case OBJECTTYPE_AXIS_UNITLABEL:
                    break;
            case OBJECTTYPE_DATA_LABELS:
            case OBJECTTYPE_DATA_SERIES:
            case OBJECTTYPE_DATA_LABEL:
            case OBJECTTYPE_DATA_POINT:
            {
                ::std::auto_ptr< awt::Size > pRefSize;
                if( pRefSizeProvider.get() )
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getPageSize()));

                wrapper::GraphicPropertyItemConverter::eGraphicObjectType eMapTo =
                    wrapper::GraphicPropertyItemConverter::FILLED_DATA_POINT;

                uno::Reference< XDataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( aObjectCID, xChartModel );
                uno::Reference< XChartType > xChartType = ChartModelHelper::getChartTypeOfSeries( xChartModel, xSeries );

                uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
                sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );
                if( !ChartTypeHelper::isSupportingAreaProperties( xChartType, nDimensionCount ) )
                    eMapTo = wrapper::GraphicPropertyItemConverter::LINE_DATA_POINT;

                bool bDataSeries = ( eObjectType == OBJECTTYPE_DATA_SERIES || eObjectType == OBJECTTYPE_DATA_LABELS );

                //special color for pie chart:
                bool bUseSpecialFillColor = false;
                sal_Int32 nSpecialFillColor =0;
                sal_Int32 nPointIndex = -1; /*-1 for whole series*/
                if(!bDataSeries)
                {
                    nPointIndex = aParticleID.toInt32();
                    uno::Reference< beans::XPropertySet > xSeriesProp( xSeries, uno::UNO_QUERY );
                    bool bVaryColorsByPoint = false;
                    if( xSeriesProp.is() &&
                        (xSeriesProp->getPropertyValue("VaryColorsByPoint") >>= bVaryColorsByPoint) &&
                        bVaryColorsByPoint )
                    {
                        if( !ColorPerPointHelper::hasPointOwnColor( xSeriesProp, nPointIndex, xObjectProperties ) )
                        {
                            bUseSpecialFillColor = true;
                            OSL_ASSERT( xDiagram.is());
                            uno::Reference< XColorScheme > xColorScheme( xDiagram->getDefaultColorScheme() );
                            if( xColorScheme.is())
                                nSpecialFillColor = xColorScheme->getColorByIndex( nPointIndex );
                        }
                    }
                }
                sal_Int32 nNumberFormat=ExplicitValueProvider::getExplicitNumberFormatKeyForDataLabel( xObjectProperties, xSeries, nPointIndex, xDiagram );
                sal_Int32 nPercentNumberFormat=ExplicitValueProvider::getExplicitPercentageNumberFormatKeyForDataLabel(
                        xObjectProperties,uno::Reference< util::XNumberFormatsSupplier >(xChartModel, uno::UNO_QUERY));

                pItemConverter =  new wrapper::DataPointItemConverter( xChartModel, xContext,
                                        xObjectProperties, xSeries, rDrawModel.GetItemPool(), rDrawModel,
                                        uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                        eMapTo, pRefSize, bDataSeries, bUseSpecialFillColor, nSpecialFillColor, true,
                                        nNumberFormat, nPercentNumberFormat );
                    break;
            }
            case OBJECTTYPE_GRID:
            case OBJECTTYPE_SUBGRID:
            case OBJECTTYPE_DATA_AVERAGE_LINE:
                pItemConverter =  new wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(),
                                        rDrawModel, uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                        wrapper::GraphicPropertyItemConverter::LINE_PROPERTIES );
                    break;

            case OBJECTTYPE_DATA_ERRORS_X:
            case OBJECTTYPE_DATA_ERRORS_Y:
            case OBJECTTYPE_DATA_ERRORS_Z:
                pItemConverter =  new wrapper::ErrorBarItemConverter(
                    xChartModel, xObjectProperties, rDrawModel.GetItemPool(),
                    rDrawModel, uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ));
                break;

            case OBJECTTYPE_DATA_CURVE:
                pItemConverter =  new wrapper::RegressionCurveItemConverter(
                    xObjectProperties, uno::Reference< chart2::XRegressionCurveContainer >(
                        ObjectIdentifier::getDataSeriesForCID( aObjectCID, xChartModel ), uno::UNO_QUERY ),
                    rDrawModel.GetItemPool(), rDrawModel,
                    uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ));
                break;
            case OBJECTTYPE_DATA_CURVE_EQUATION:
            {
                ::std::auto_ptr< awt::Size > pRefSize;
                if( pRefSizeProvider.get() )
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getPageSize()));

                pItemConverter =  new wrapper::RegressionEquationItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(), rDrawModel,
                                        uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                        pRefSize );
                    break;
            }
            case OBJECTTYPE_DATA_STOCK_RANGE:
                    break;
            case OBJECTTYPE_DATA_STOCK_LOSS:
            case OBJECTTYPE_DATA_STOCK_GAIN:
                pItemConverter =  new wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(),
                                        rDrawModel, uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ),
                                        wrapper::GraphicPropertyItemConverter::LINE_AND_FILL_PROPERTIES );
                    break;
            default: //OBJECTTYPE_UNKNOWN
                    break;
        }
    }
    else
    {
        //create itemconverter for a all objects of given type
        switch(eObjectType)
        {
            case OBJECTTYPE_TITLE:
                pItemConverter =  new wrapper::AllTitleItemConverter( xChartModel, rDrawModel.GetItemPool(),
                                                                     rDrawModel, uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ));
                break;
            case OBJECTTYPE_AXIS:
            {
                ::std::auto_ptr< awt::Size > pRefSize;
                if( pRefSizeProvider.get() )
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getPageSize()));

                pItemConverter =  new wrapper::AllAxisItemConverter( xChartModel, rDrawModel.GetItemPool(),
                                                                     rDrawModel, uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ), pRefSize );
            }
            break;
            case OBJECTTYPE_GRID:
            case OBJECTTYPE_SUBGRID:
                pItemConverter =  new wrapper::AllGridItemConverter( xChartModel, rDrawModel.GetItemPool(),
                                                                     rDrawModel, uno::Reference< lang::XMultiServiceFactory >( xChartModel, uno::UNO_QUERY ));
                break;
            default: //for this type it is not supported to change all elements at once
                break;
        }

    }
    return pItemConverter;
}
SAL_WNODEPRECATED_DECLARATIONS_POP

OUString lcl_getTitleCIDForCommand( const OString& rDispatchCommand, const uno::Reference< frame::XModel > & xChartModel )
{
    if( rDispatchCommand.equals("AllTitles"))
        return ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_TITLE, "ALLELEMENTS" );

    TitleHelper::eTitleType nTitleType( TitleHelper::MAIN_TITLE );
    if( rDispatchCommand.equals("SubTitle") )
        nTitleType = TitleHelper::SUB_TITLE;
    else if( rDispatchCommand.equals("XTitle") )
        nTitleType = TitleHelper::X_AXIS_TITLE;
    else if( rDispatchCommand.equals("YTitle") )
        nTitleType = TitleHelper::Y_AXIS_TITLE;
    else if( rDispatchCommand.equals("ZTitle") )
        nTitleType = TitleHelper::Z_AXIS_TITLE;
    else if( rDispatchCommand.equals("SecondaryXTitle") )
        nTitleType = TitleHelper::SECONDARY_X_AXIS_TITLE;
    else if( rDispatchCommand.equals("SecondaryYTitle") )
        nTitleType = TitleHelper::SECONDARY_Y_AXIS_TITLE;

    uno::Reference< XTitle > xTitle( TitleHelper::getTitle( nTitleType, xChartModel ) );
    return ObjectIdentifier::createClassifiedIdentifierForObject( xTitle, xChartModel );
}

OUString lcl_getAxisCIDForCommand( const OString& rDispatchCommand, const uno::Reference< frame::XModel >& xChartModel )
{
    if( rDispatchCommand.equals("DiagramAxisAll"))
        return ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_AXIS, "ALLELEMENTS" );

    sal_Int32   nDimensionIndex=0;
    bool        bMainAxis=true;
    if( rDispatchCommand.equals("DiagramAxisX"))
    {
        nDimensionIndex=0; bMainAxis=true;
    }
    else if( rDispatchCommand.equals("DiagramAxisY"))
    {
        nDimensionIndex=1; bMainAxis=true;
    }
    else if( rDispatchCommand.equals("DiagramAxisZ"))
    {
        nDimensionIndex=2; bMainAxis=true;
    }
    else if( rDispatchCommand.equals("DiagramAxisA"))
    {
        nDimensionIndex=0; bMainAxis=false;
    }
    else if( rDispatchCommand.equals("DiagramAxisB"))
    {
        nDimensionIndex=1; bMainAxis=false;
    }

    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    uno::Reference< XAxis > xAxis( AxisHelper::getAxis( nDimensionIndex, bMainAxis, xDiagram ) );
    return ObjectIdentifier::createClassifiedIdentifierForObject( xAxis, xChartModel );
}

OUString lcl_getGridCIDForCommand( const OString& rDispatchCommand, const uno::Reference< frame::XModel >& xChartModel )
{
    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );

    if( rDispatchCommand.equals("DiagramGridAll"))
        return ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_GRID, "ALLELEMENTS" );

    sal_Int32   nDimensionIndex=0;
    bool        bMainGrid=true;

    //x and y is swapped in the commands

    if( rDispatchCommand.equals("DiagramGridYMain"))
    {
        nDimensionIndex=0; bMainGrid=true;
    }
    else if( rDispatchCommand.equals("DiagramGridXMain"))
    {
        nDimensionIndex=1; bMainGrid=true;
    }
    else if( rDispatchCommand.equals("DiagramGridZMain"))
    {
        nDimensionIndex=2; bMainGrid=true;
    }
    else if( rDispatchCommand.equals("DiagramGridYHelp"))
    {
        nDimensionIndex=0; bMainGrid=false;
    }
    else if( rDispatchCommand.equals("DiagramGridXHelp"))
    {
        nDimensionIndex=1; bMainGrid=false;
    }
    else if( rDispatchCommand.equals("DiagramGridZHelp"))
    {
        nDimensionIndex=2; bMainGrid=false;
    }

    bool bMainAxis = true;
    uno::Reference< XAxis > xAxis( AxisHelper::getAxis( nDimensionIndex, bMainAxis, xDiagram ) );

    sal_Int32   nSubGridIndex= bMainGrid ? (-1) : 0;
    OUString aCID( ObjectIdentifier::createClassifiedIdentifierForGrid( xAxis, xChartModel, nSubGridIndex ) );
    return aCID;
}

OUString lcl_getErrorCIDForCommand( const ObjectType eDispatchType, const ObjectType &eSelectedType, const OUString &rSelectedCID)
{
    if( eSelectedType == eDispatchType )
        return rSelectedCID;

    return ObjectIdentifier::createClassifiedIdentifierWithParent( eDispatchType, OUString(), rSelectedCID );
}

OUString lcl_getObjectCIDForCommand( const OString& rDispatchCommand, const uno::Reference< XChartDocument > & xChartDocument, const OUString& rSelectedCID )
{
    ObjectType eObjectType = OBJECTTYPE_UNKNOWN;
    OUString aParticleID;

    uno::Reference< frame::XModel > xChartModel( xChartDocument, uno::UNO_QUERY );
    const ObjectType eSelectedType = ObjectIdentifier::getObjectType( rSelectedCID );
    uno::Reference< XDataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( rSelectedCID, xChartModel );
    uno::Reference< chart2::XRegressionCurveContainer > xRegCurveCnt( xSeries, uno::UNO_QUERY );

    //legend
    if( rDispatchCommand.equals("Legend") || rDispatchCommand.equals("FormatLegend") )
    {
        eObjectType = OBJECTTYPE_LEGEND;
        //@todo set particular aParticleID if we have more than one legend
    }
    //wall floor area
    else if( rDispatchCommand.equals("DiagramWall") || rDispatchCommand.equals("FormatWall") )
    {
        //OBJECTTYPE_DIAGRAM;
        eObjectType = OBJECTTYPE_DIAGRAM_WALL;
        //@todo set particular aParticleID if we have more than one diagram
    }
    else if( rDispatchCommand.equals("DiagramFloor") || rDispatchCommand.equals("FormatFloor") )
    {
        eObjectType = OBJECTTYPE_DIAGRAM_FLOOR;
        //@todo set particular aParticleID if we have more than one diagram
    }
    else if( rDispatchCommand.equals("DiagramArea") || rDispatchCommand.equals("FormatChartArea") )
    {
        eObjectType = OBJECTTYPE_PAGE;
    }
    //title
    else if( rDispatchCommand.equals("MainTitle")
        || rDispatchCommand.equals("SubTitle")
        || rDispatchCommand.equals("XTitle")
        || rDispatchCommand.equals("YTitle")
        || rDispatchCommand.equals("ZTitle")
        || rDispatchCommand.equals("SecondaryXTitle")
        || rDispatchCommand.equals("SecondaryYTitle")
        || rDispatchCommand.equals("AllTitles")
        )
    {
        return lcl_getTitleCIDForCommand( rDispatchCommand, xChartModel );
    }
    //axis
    else if( rDispatchCommand.equals("DiagramAxisX")
        || rDispatchCommand.equals("DiagramAxisY")
        || rDispatchCommand.equals("DiagramAxisZ")
        || rDispatchCommand.equals("DiagramAxisA")
        || rDispatchCommand.equals("DiagramAxisB")
        || rDispatchCommand.equals("DiagramAxisAll")
        )
    {
        return lcl_getAxisCIDForCommand( rDispatchCommand, xChartModel );
    }
    //grid
    else if( rDispatchCommand.equals("DiagramGridYMain")
        || rDispatchCommand.equals("DiagramGridXMain")
        || rDispatchCommand.equals("DiagramGridZMain")
        || rDispatchCommand.equals("DiagramGridYHelp")
        || rDispatchCommand.equals("DiagramGridXHelp")
        || rDispatchCommand.equals("DiagramGridZHelp")
        || rDispatchCommand.equals("DiagramGridAll")
        )
    {
        return lcl_getGridCIDForCommand( rDispatchCommand, xChartModel );
    }
    //data series
    else if( rDispatchCommand.equals("FormatDataSeries") )
    {
        if( eSelectedType == OBJECTTYPE_DATA_SERIES )
            return rSelectedCID;
        else
            return ObjectIdentifier::createClassifiedIdentifier(
                OBJECTTYPE_DATA_SERIES, ObjectIdentifier::getSeriesParticleFromCID( rSelectedCID ) );
    }
    //data point
    else if( rDispatchCommand.equals("FormatDataPoint") )
    {
        return rSelectedCID;
    }
    //data labels
    else if( rDispatchCommand.equals("FormatDataLabels") )
    {
        if( eSelectedType == OBJECTTYPE_DATA_LABELS )
            return rSelectedCID;
        else
            return ObjectIdentifier::createClassifiedIdentifierWithParent(
                OBJECTTYPE_DATA_LABELS, OUString(), rSelectedCID );
    }
    //data labels
    else if( rDispatchCommand.equals("FormatDataLabel") )
    {
        if( eSelectedType == OBJECTTYPE_DATA_LABEL )
            return rSelectedCID;
        else
        {
            sal_Int32 nPointIndex = ObjectIdentifier::getParticleID( rSelectedCID ).toInt32();
            if( nPointIndex>=0 )
            {
                OUString aSeriesParticle = ObjectIdentifier::getSeriesParticleFromCID( rSelectedCID );
                OUString aChildParticle( ObjectIdentifier::getStringForType( OBJECTTYPE_DATA_LABELS ) + "=" );
                OUString aLabelsCID = ObjectIdentifier::createClassifiedIdentifierForParticles( aSeriesParticle, aChildParticle );
                OUString aLabelCID_Stub = ObjectIdentifier::createClassifiedIdentifierWithParent(
                    OBJECTTYPE_DATA_LABEL, OUString(), aLabelsCID );

                return ObjectIdentifier::createPointCID( aLabelCID_Stub, nPointIndex );
            }
        }
    }
    //mean value line
    else if( rDispatchCommand.equals("FormatMeanValue") )
    {
        if( eSelectedType == OBJECTTYPE_DATA_AVERAGE_LINE )
            return rSelectedCID;
        else
            return ObjectIdentifier::createDataCurveCID(
                ObjectIdentifier::getSeriesParticleFromCID( rSelectedCID ),
                    RegressionCurveHelper::getRegressionCurveIndex( xRegCurveCnt,
                        RegressionCurveHelper::getMeanValueLine( xRegCurveCnt ) ), true );
    }
    //trend line
    else if( rDispatchCommand.equals("FormatTrendline") )
    {
        if( eSelectedType == OBJECTTYPE_DATA_CURVE )
            return rSelectedCID;
        else
            return ObjectIdentifier::createDataCurveCID(
                ObjectIdentifier::getSeriesParticleFromCID( rSelectedCID ),
                    RegressionCurveHelper::getRegressionCurveIndex( xRegCurveCnt,
                        RegressionCurveHelper::getFirstCurveNotMeanValueLine( xRegCurveCnt ) ), false );
    }
    //trend line equation
    else if( rDispatchCommand.equals("FormatTrendlineEquation") )
    {
        if( eSelectedType == OBJECTTYPE_DATA_CURVE_EQUATION )
            return rSelectedCID;
        else
            return ObjectIdentifier::createDataCurveEquationCID(
                ObjectIdentifier::getSeriesParticleFromCID( rSelectedCID ),
                    RegressionCurveHelper::getRegressionCurveIndex( xRegCurveCnt,
                        RegressionCurveHelper::getFirstCurveNotMeanValueLine( xRegCurveCnt ) ) );
    }
    // y error bars
    else if( rDispatchCommand.equals("FormatXErrorBars") )
    {
        return lcl_getErrorCIDForCommand(OBJECTTYPE_DATA_ERRORS_X, eSelectedType, rSelectedCID );
    }
    // y error bars
    else if( rDispatchCommand.equals("FormatYErrorBars") )
    {
        return lcl_getErrorCIDForCommand(OBJECTTYPE_DATA_ERRORS_Y, eSelectedType, rSelectedCID );
    }
    // axis
    else if( rDispatchCommand.equals("FormatAxis") )
    {
        if( eSelectedType == OBJECTTYPE_AXIS )
            return rSelectedCID;
        else
        {
            Reference< XAxis > xAxis = ObjectIdentifier::getAxisForCID( rSelectedCID, xChartModel );
            return ObjectIdentifier::createClassifiedIdentifierForObject( xAxis , xChartModel );
        }
    }
    // major grid
    else if( rDispatchCommand.equals("FormatMajorGrid") )
    {
        if( eSelectedType == OBJECTTYPE_GRID )
            return rSelectedCID;
        else
        {
            Reference< XAxis > xAxis = ObjectIdentifier::getAxisForCID( rSelectedCID, xChartModel );
            return ObjectIdentifier::createClassifiedIdentifierForGrid( xAxis, xChartModel );
        }

    }
    // minor grid
    else if( rDispatchCommand.equals("FormatMinorGrid") )
    {
        if( eSelectedType == OBJECTTYPE_SUBGRID )
            return rSelectedCID;
        else
        {
            Reference< XAxis > xAxis = ObjectIdentifier::getAxisForCID( rSelectedCID, xChartModel );
            return ObjectIdentifier::createClassifiedIdentifierForGrid( xAxis, xChartModel, 0 /*sub grid index*/ );
        }
    }
    // title
    else if( rDispatchCommand.equals("FormatTitle") )
    {
        if( eSelectedType == OBJECTTYPE_TITLE )
            return rSelectedCID;
    }
    // stock loss
    else if( rDispatchCommand.equals("FormatStockLoss") )
    {
        if( eSelectedType == OBJECTTYPE_DATA_STOCK_LOSS )
            return rSelectedCID;
        else
            return ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DATA_STOCK_LOSS, OUString());
    }
    // stock gain
    else if( rDispatchCommand.equals("FormatStockGain") )
    {
        if( eSelectedType == OBJECTTYPE_DATA_STOCK_GAIN )
            return rSelectedCID;
        else
            return ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DATA_STOCK_GAIN, OUString() );
    }

    return ObjectIdentifier::createClassifiedIdentifier(
        eObjectType, aParticleID );
}

}
// anonymous namespace

void SAL_CALL ChartController::executeDispatch_FormatObject(const OUString& rDispatchCommand)
{
    uno::Reference< XChartDocument > xChartDocument( getModel(), uno::UNO_QUERY );
    OString aCommand( OUStringToOString( rDispatchCommand, RTL_TEXTENCODING_ASCII_US ) );
    OUString rObjectCID = lcl_getObjectCIDForCommand( aCommand, xChartDocument, m_aSelection.getSelectedCID() );
    executeDlg_ObjectProperties( rObjectCID );
}

void SAL_CALL ChartController::executeDispatch_ObjectProperties()
{
    executeDlg_ObjectProperties( m_aSelection.getSelectedCID() );
}

namespace
{

OUString lcl_getFormatCIDforSelectedCID( const OUString& rSelectedCID )
{
    OUString aFormatCID(rSelectedCID);

    //get type of selected object
    ObjectType eObjectType = ObjectIdentifier::getObjectType( aFormatCID );

    // some legend entries are handled as if they were data series
    if( OBJECTTYPE_LEGEND_ENTRY==eObjectType )
    {
        OUString aParentParticle( ObjectIdentifier::getFullParentParticle( rSelectedCID ) );
        aFormatCID  = ObjectIdentifier::createClassifiedIdentifierForParticle( aParentParticle );
    }

    // treat diagram as wall
    if( OBJECTTYPE_DIAGRAM==eObjectType )
        aFormatCID  = ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM_WALL, OUString() );

    return aFormatCID;
}

}//end anonymous namespace

void SAL_CALL ChartController::executeDlg_ObjectProperties( const OUString& rSelectedObjectCID )
{
    OUString aObjectCID = lcl_getFormatCIDforSelectedCID( rSelectedObjectCID );

    UndoGuard aUndoGuard( ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::FORMAT,
                ObjectNameProvider::getName( ObjectIdentifier::getObjectType( aObjectCID ))),
            m_xUndoManager );

    bool bSuccess = ChartController::executeDlg_ObjectProperties_withoutUndoGuard( aObjectCID, false );
    if( bSuccess )
        aUndoGuard.commit();
}

bool ChartController::executeDlg_ObjectProperties_withoutUndoGuard( const OUString& rObjectCID, bool bOkClickOnUnchangedDialogSouldBeRatedAsSuccessAlso )
{
    //return true if the properties were changed successfully
    bool bRet = false;
    if( rObjectCID.isEmpty() )
    {
       return bRet;
    }
    try
    {
        //get type of object
        ObjectType eObjectType = ObjectIdentifier::getObjectType( rObjectCID );
        if( OBJECTTYPE_UNKNOWN==eObjectType )
        {
            return bRet;
        }
        if( OBJECTTYPE_DIAGRAM_WALL==eObjectType || OBJECTTYPE_DIAGRAM_FLOOR==eObjectType )
        {
            if( !DiagramHelper::isSupportingFloorAndWall( ChartModelHelper::findDiagram( getModel() ) ) )
                return bRet;
        }

        //convert properties to ItemSet

        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr< ReferenceSizeProvider > pRefSizeProv(
            impl_createReferenceSizeProvider());
        ::std::auto_ptr< ::comphelper::ItemConverter > apItemConverter(
            createItemConverter( rObjectCID, getModel(), m_xCC,
                                 m_pDrawModelWrapper->getSdrModel(),
                                 ExplicitValueProvider::getExplicitValueProvider(m_xChartView),
                                 pRefSizeProv ));
        SAL_WNODEPRECATED_DECLARATIONS_POP
        if(!apItemConverter.get())
            return bRet;

        SfxItemSet aItemSet = apItemConverter->CreateEmptyItemSet();

        if ( eObjectType == OBJECTTYPE_DATA_ERRORS_X || eObjectType == OBJECTTYPE_DATA_ERRORS_Y )
            aItemSet.Put(SfxBoolItem(SCHATTR_STAT_ERRORBAR_TYPE, eObjectType == OBJECTTYPE_DATA_ERRORS_Y ));

        apItemConverter->FillItemSet( aItemSet );

        //prepare dialog
        ObjectPropertiesDialogParameter aDialogParameter = ObjectPropertiesDialogParameter( rObjectCID );
        aDialogParameter.init( getModel() );
        ViewElementListProvider aViewElementListProvider( m_pDrawModelWrapper.get() );

        SolarMutexGuard aGuard;
        SchAttribTabDlg aDlg( m_pChartWindow, &aItemSet, &aDialogParameter, &aViewElementListProvider
            , uno::Reference< util::XNumberFormatsSupplier >( getModel(), uno::UNO_QUERY ) );

        if(aDialogParameter.HasSymbolProperties())
        {
            SfxItemSet* pSymbolShapeProperties=NULL;
            uno::Reference< beans::XPropertySet > xObjectProperties =
                ObjectIdentifier::getObjectPropertySet( rObjectCID, getModel() );
            wrapper::DataPointItemConverter aSymbolItemConverter( getModel(), m_xCC
                                        , xObjectProperties, ObjectIdentifier::getDataSeriesForCID( rObjectCID, getModel() )
                                        , m_pDrawModelWrapper->getSdrModel().GetItemPool()
                                        , m_pDrawModelWrapper->getSdrModel()
                                        , uno::Reference< lang::XMultiServiceFactory >( getModel(), uno::UNO_QUERY )
                                        , wrapper::GraphicPropertyItemConverter::FILLED_DATA_POINT );

            pSymbolShapeProperties = new SfxItemSet( aSymbolItemConverter.CreateEmptyItemSet() );
            aSymbolItemConverter.FillItemSet( *pSymbolShapeProperties );

            sal_Int32   nStandardSymbol=0;//@todo get from somewhere
            Graphic*    pAutoSymbolGraphic = new Graphic( aViewElementListProvider.GetSymbolGraphic( nStandardSymbol, pSymbolShapeProperties ) );
            // note: the dialog takes the ownership of pSymbolShapeProperties and pAutoSymbolGraphic
            aDlg.setSymbolInformation( pSymbolShapeProperties, pAutoSymbolGraphic );
        }
        if( aDialogParameter.HasStatisticProperties() )
        {
            aDlg.SetAxisMinorStepWidthForErrorBarDecimals(
                InsertErrorBarsDialog::getAxisMinorStepWidthForErrorBarDecimals( getModel(), m_xChartView, rObjectCID ) );
        }

        //open the dialog
        if( aDlg.Execute() == RET_OK || (bOkClickOnUnchangedDialogSouldBeRatedAsSuccessAlso && aDlg.DialogWasClosedWithOK()) )
        {
            const SfxItemSet* pOutItemSet = aDlg.GetOutputItemSet();
            if(pOutItemSet)
            {
                ControllerLockGuard aCLGuard( getModel());
                apItemConverter->ApplyItemSet( *pOutItemSet );//model should be changed now
                bRet = true;
            }
        }
    }
    catch( const util::CloseVetoException& )
    {
    }
    catch( const uno::RuntimeException& )
    {
    }
    return bRet;
}

void SAL_CALL ChartController::executeDispatch_View3D()
{
    try
    {
        // using assignment for broken gcc 3.3
        UndoLiveUpdateGuard aUndoGuard = UndoLiveUpdateGuard(
            SCH_RESSTR( STR_ACTION_EDIT_3D_VIEW ),
            m_xUndoManager );

        //open dialog
        SolarMutexGuard aSolarGuard;
        View3DDialog aDlg( m_pChartWindow, getModel(), m_pDrawModelWrapper->GetColorList() );
        if( aDlg.Execute() == RET_OK )
            aUndoGuard.commit();
    }
    catch(const uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
