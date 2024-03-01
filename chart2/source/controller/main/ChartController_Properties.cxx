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

#include <ChartController.hxx>
#include <chartview/DrawModelWrapper.hxx>
#include <chartview/ChartSfxItemIds.hxx>
#include <ObjectIdentifier.hxx>
#include <chartview/ExplicitScaleValues.hxx>
#include <chartview/ExplicitValueProvider.hxx>
#include <dlg_ObjectProperties.hxx>
#include <dlg_View3D.hxx>
#include <dlg_InsertErrorBars.hxx>
#include <ViewElementListProvider.hxx>
#include <DataPointItemConverter.hxx>
#include <TextLabelItemConverter.hxx>
#include <AxisItemConverter.hxx>
#include <MultipleChartConverters.hxx>
#include <TitleItemConverter.hxx>
#include <LegendItemConverter.hxx>
#include <DataTableItemConverter.hxx>
#include <RegressionCurveItemConverter.hxx>
#include <RegressionEquationItemConverter.hxx>
#include <ErrorBarItemConverter.hxx>
#include <ChartModelHelper.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <TitleHelper.hxx>
#include <ChartType.hxx>
#include <ChartTypeHelper.hxx>
#include <ChartModel.hxx>
#include <ColorPerPointHelper.hxx>
#include <DataSeries.hxx>
#include <DiagramHelper.hxx>
#include <Diagram.hxx>
#include <ControllerLockGuard.hxx>
#include "UndoGuard.hxx"
#include <ObjectNameProvider.hxx>
#include <ResId.hxx>
#include <strings.hrc>
#include <ReferenceSizeProvider.hxx>
#include <RegressionCurveHelper.hxx>
#include <RegressionCurveModel.hxx>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <comphelper/servicehelper.hxx>
#include <o3tl/string_view.hxx>

#include <memory>

#include <vcl/svapp.hxx>
#include <svx/ActionDescriptionProvider.hxx>
#include <comphelper/diagnose_ex.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

namespace
{

wrapper::ItemConverter* createItemConverter(
    std::u16string_view aObjectCID, const rtl::Reference<::chart::ChartModel>& xChartModel,
    const uno::Reference<uno::XComponentContext>& xContext, SdrModel& rDrawModel,
    ExplicitValueProvider* pExplicitValueProvider, ReferenceSizeProvider const * pRefSizeProvider )
{
    wrapper::ItemConverter* pItemConverter=nullptr;

    //get type of selected object
    ObjectType eObjectType = ObjectIdentifier::getObjectType( aObjectCID );
    if( eObjectType==OBJECTTYPE_UNKNOWN )
    {
        OSL_FAIL("unknown ObjectType");
        return nullptr;
    }

    std::u16string_view aParticleID = ObjectIdentifier::getParticleID( aObjectCID );
    bool bAffectsMultipleObjects = aParticleID == u"ALLELEMENTS";
    if( !bAffectsMultipleObjects )
    {
        uno::Reference< beans::XPropertySet > xObjectProperties =
            ObjectIdentifier::getObjectPropertySet( aObjectCID, xChartModel );
        if(!xObjectProperties.is())
            return nullptr;
        //create itemconverter for a single object
        switch(eObjectType)
        {
            case OBJECTTYPE_PAGE:
                pItemConverter =  new wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(),
                                        rDrawModel, xChartModel,
                                        wrapper::GraphicObjectType::LineAndFillProperties );
                    break;
            case OBJECTTYPE_TITLE:
            {
                std::unique_ptr<awt::Size> pRefSize;
                if (pRefSizeProvider)
                    pRefSize.reset(new awt::Size(pRefSizeProvider->getPageSize()));

                pItemConverter = new wrapper::TitleItemConverter(
                    xObjectProperties, rDrawModel.GetItemPool(), rDrawModel,
                    xChartModel,
                    pRefSize.get());
            }
            break;
            case OBJECTTYPE_LEGEND:
            {
                std::unique_ptr<awt::Size> pRefSize;
                if (pRefSizeProvider)
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getPageSize()));

                pItemConverter = new wrapper::LegendItemConverter(
                    xObjectProperties, rDrawModel.GetItemPool(), rDrawModel,
                    xChartModel,
                    pRefSize.get());
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
                                        rDrawModel, xChartModel,
                                        wrapper::GraphicObjectType::LineAndFillProperties );
                    break;
            case OBJECTTYPE_AXIS:
            {
                std::unique_ptr<awt::Size> pRefSize;
                if (pRefSizeProvider)
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getPageSize()));

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
                    xChartModel,
                    &aExplicitScale, &aExplicitIncrement,
                    pRefSize.get() );
            }
            break;
            case OBJECTTYPE_AXIS_UNITLABEL:
                    break;
            case OBJECTTYPE_DATA_LABELS:
            case OBJECTTYPE_DATA_LABEL:
            {
                std::unique_ptr<awt::Size> pRefSize;
                if (pRefSizeProvider)
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getPageSize()));

                rtl::Reference<DataSeries> xSeries = ObjectIdentifier::getDataSeriesForCID(aObjectCID, xChartModel);

                bool bDataSeries = eObjectType == OBJECTTYPE_DATA_LABELS;

                sal_Int32 nNumberFormat = ExplicitValueProvider::getExplicitNumberFormatKeyForDataLabel( xObjectProperties );
                sal_Int32 nPercentNumberFormat = ExplicitValueProvider::getExplicitPercentageNumberFormatKeyForDataLabel(
                    xObjectProperties, xChartModel);

                pItemConverter = new wrapper::TextLabelItemConverter(
                    xChartModel, xObjectProperties, xSeries,
                    rDrawModel.GetItemPool(), pRefSize.get(), bDataSeries,
                    nNumberFormat, nPercentNumberFormat);
            }
            break;
            case OBJECTTYPE_DATA_SERIES:
            case OBJECTTYPE_DATA_POINT:
            {
                std::unique_ptr<awt::Size> pRefSize;
                if (pRefSizeProvider)
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getPageSize()));

                wrapper::GraphicObjectType eMapTo =
                    wrapper::GraphicObjectType::FilledDataPoint;

                rtl::Reference< DataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( aObjectCID, xChartModel );
                rtl::Reference< ChartType > xChartType = ChartModelHelper::getChartTypeOfSeries( xChartModel, xSeries );

                rtl::Reference< Diagram > xDiagram = ChartModelHelper::findDiagram( xChartModel );
                sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );
                if( !ChartTypeHelper::isSupportingAreaProperties( xChartType, nDimensionCount ) )
                    eMapTo = wrapper::GraphicObjectType::LineDataPoint;

                bool bDataSeries = eObjectType == OBJECTTYPE_DATA_SERIES;

                //special color for pie chart:
                bool bUseSpecialFillColor = false;
                sal_Int32 nSpecialFillColor =0;
                sal_Int32 nPointIndex = -1; /*-1 for whole series*/
                if(!bDataSeries)
                {
                    nPointIndex = o3tl::toInt32(aParticleID);
                    bool bVaryColorsByPoint = false;
                    if( xSeries.is() &&
                        (xSeries->getPropertyValue("VaryColorsByPoint") >>= bVaryColorsByPoint) &&
                        bVaryColorsByPoint )
                    {
                        if( !ColorPerPointHelper::hasPointOwnColor( xSeries, nPointIndex, xObjectProperties ) )
                        {
                            bUseSpecialFillColor = true;
                            OSL_ASSERT( xDiagram.is());
                            uno::Reference< XColorScheme > xColorScheme( xDiagram->getDefaultColorScheme() );
                            if( xColorScheme.is())
                                nSpecialFillColor = xColorScheme->getColorByIndex( nPointIndex );
                        }
                    }
                }
                sal_Int32 nNumberFormat=ExplicitValueProvider::getExplicitNumberFormatKeyForDataLabel( xObjectProperties );
                sal_Int32 nPercentNumberFormat=ExplicitValueProvider::getExplicitPercentageNumberFormatKeyForDataLabel(
                        xObjectProperties, xChartModel);

                pItemConverter =  new wrapper::DataPointItemConverter( xChartModel, xContext,
                                        xObjectProperties, xSeries, rDrawModel.GetItemPool(), rDrawModel,
                                        xChartModel,
                                        eMapTo, pRefSize.get(), bDataSeries, bUseSpecialFillColor, nSpecialFillColor, true,
                                        nNumberFormat, nPercentNumberFormat, nPointIndex );
                break;
            }
            case OBJECTTYPE_GRID:
            case OBJECTTYPE_SUBGRID:
            case OBJECTTYPE_DATA_AVERAGE_LINE:
                pItemConverter =  new wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(),
                                        rDrawModel, xChartModel,
                                        wrapper::GraphicObjectType::LineProperties );
                    break;

            case OBJECTTYPE_DATA_ERRORS_X:
            case OBJECTTYPE_DATA_ERRORS_Y:
            case OBJECTTYPE_DATA_ERRORS_Z:
                pItemConverter =  new wrapper::ErrorBarItemConverter(
                    xChartModel, xObjectProperties, rDrawModel.GetItemPool(),
                    rDrawModel, xChartModel);
                break;

            case OBJECTTYPE_DATA_CURVE:
                pItemConverter =  new wrapper::RegressionCurveItemConverter(
                        xObjectProperties,
                        ObjectIdentifier::getDataSeriesForCID( aObjectCID, xChartModel ),
                        rDrawModel.GetItemPool(), rDrawModel,
                        xChartModel);
                break;
            case OBJECTTYPE_DATA_CURVE_EQUATION:
            {
                std::unique_ptr<awt::Size> pRefSize;
                if (pRefSizeProvider)
                    pRefSize.reset(new awt::Size(pRefSizeProvider->getPageSize()));

                pItemConverter =  new wrapper::RegressionEquationItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(), rDrawModel,
                                        xChartModel,
                                        pRefSize.get());
                break;
            }
            case OBJECTTYPE_DATA_STOCK_RANGE:
                    break;
            case OBJECTTYPE_DATA_STOCK_LOSS:
            case OBJECTTYPE_DATA_STOCK_GAIN:
                pItemConverter =  new wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(),
                                        rDrawModel, xChartModel,
                                        wrapper::GraphicObjectType::LineAndFillProperties );
                    break;
            case OBJECTTYPE_DATA_TABLE:
            {
                pItemConverter =  new wrapper::DataTableItemConverter(
                                        xObjectProperties, rDrawModel.GetItemPool(),
                                        rDrawModel, xChartModel);
            }
            break;
            default: //OBJECTTYPE_UNKNOWN
                    break;
        }
    }
    else
    {
        //create itemconverter for all objects of given type
        switch(eObjectType)
        {
            case OBJECTTYPE_TITLE:
                pItemConverter =  new wrapper::AllTitleItemConverter( xChartModel, rDrawModel.GetItemPool(),
                                                                     rDrawModel, xChartModel);
                break;
            case OBJECTTYPE_AXIS:
            {
                std::unique_ptr<awt::Size> pRefSize;
                if (pRefSizeProvider)
                    pRefSize.reset( new awt::Size( pRefSizeProvider->getPageSize()));

                pItemConverter =  new wrapper::AllAxisItemConverter(
                    xChartModel, rDrawModel.GetItemPool(),
                    rDrawModel, pRefSize.get());
            }
            break;
            case OBJECTTYPE_GRID:
            case OBJECTTYPE_SUBGRID:
                pItemConverter =  new wrapper::AllGridItemConverter( xChartModel, rDrawModel.GetItemPool(),
                                                                     rDrawModel, xChartModel);
                break;
            default: //for this type it is not supported to change all elements at once
                break;
        }

    }
    return pItemConverter;
}

OUString lcl_getTitleCIDForCommand( std::string_view rDispatchCommand, const rtl::Reference<::chart::ChartModel> & xChartModel )
{
    if( rDispatchCommand == "AllTitles")
        return ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_TITLE, u"ALLELEMENTS" );

    TitleHelper::eTitleType nTitleType( TitleHelper::MAIN_TITLE );
    if( rDispatchCommand == "SubTitle" )
        nTitleType = TitleHelper::SUB_TITLE;
    else if( rDispatchCommand == "XTitle" )
        nTitleType = TitleHelper::X_AXIS_TITLE;
    else if( rDispatchCommand == "YTitle" )
        nTitleType = TitleHelper::Y_AXIS_TITLE;
    else if( rDispatchCommand == "ZTitle" )
        nTitleType = TitleHelper::Z_AXIS_TITLE;
    else if( rDispatchCommand == "SecondaryXTitle" )
        nTitleType = TitleHelper::SECONDARY_X_AXIS_TITLE;
    else if( rDispatchCommand == "SecondaryYTitle" )
        nTitleType = TitleHelper::SECONDARY_Y_AXIS_TITLE;

    uno::Reference< XTitle > xTitle( TitleHelper::getTitle( nTitleType, xChartModel ) );
    return ObjectIdentifier::createClassifiedIdentifierForObject( xTitle, xChartModel );
}

OUString lcl_getAxisCIDForCommand( std::string_view rDispatchCommand, const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    if( rDispatchCommand == "DiagramAxisAll")
        return ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_AXIS, u"ALLELEMENTS" );

    sal_Int32   nDimensionIndex=0;
    bool        bMainAxis=true;
    if( rDispatchCommand == "DiagramAxisX")
    {
        nDimensionIndex=0; bMainAxis=true;
    }
    else if( rDispatchCommand == "DiagramAxisY")
    {
        nDimensionIndex=1; bMainAxis=true;
    }
    else if( rDispatchCommand == "DiagramAxisZ")
    {
        nDimensionIndex=2; bMainAxis=true;
    }
    else if( rDispatchCommand == "DiagramAxisA")
    {
        nDimensionIndex=0; bMainAxis=false;
    }
    else if( rDispatchCommand == "DiagramAxisB")
    {
        nDimensionIndex=1; bMainAxis=false;
    }

    rtl::Reference< Diagram > xDiagram = ChartModelHelper::findDiagram( xChartModel );
    rtl::Reference< Axis > xAxis = AxisHelper::getAxis( nDimensionIndex, bMainAxis, xDiagram );
    return ObjectIdentifier::createClassifiedIdentifierForObject( xAxis, xChartModel );
}

OUString lcl_getGridCIDForCommand( std::string_view rDispatchCommand, const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    rtl::Reference< Diagram > xDiagram = ChartModelHelper::findDiagram( xChartModel );

    if( rDispatchCommand == "DiagramGridAll")
        return ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_GRID, u"ALLELEMENTS" );

    sal_Int32   nDimensionIndex=0;
    bool        bMainGrid=true;

    //x and y is swapped in the commands

    if( rDispatchCommand == "DiagramGridYMain")
    {
        nDimensionIndex=0; bMainGrid=true;
    }
    else if( rDispatchCommand == "DiagramGridXMain")
    {
        nDimensionIndex=1; bMainGrid=true;
    }
    else if( rDispatchCommand == "DiagramGridZMain")
    {
        nDimensionIndex=2; bMainGrid=true;
    }
    else if( rDispatchCommand == "DiagramGridYHelp")
    {
        nDimensionIndex=0; bMainGrid=false;
    }
    else if( rDispatchCommand == "DiagramGridXHelp")
    {
        nDimensionIndex=1; bMainGrid=false;
    }
    else if( rDispatchCommand == "DiagramGridZHelp")
    {
        nDimensionIndex=2; bMainGrid=false;
    }

    rtl::Reference< Axis > xAxis = AxisHelper::getAxis( nDimensionIndex, true/*bMainAxis*/, xDiagram );

    sal_Int32   nSubGridIndex= bMainGrid ? -1 : 0;
    OUString aCID( ObjectIdentifier::createClassifiedIdentifierForGrid( xAxis, xChartModel, nSubGridIndex ) );
    return aCID;
}

OUString lcl_getErrorCIDForCommand( const ObjectType eDispatchType, const ObjectType &eSelectedType, const OUString &rSelectedCID)
{
    if( eSelectedType == eDispatchType )
        return rSelectedCID;

    return ObjectIdentifier::createClassifiedIdentifierWithParent( eDispatchType, u"", rSelectedCID );
}

OUString lcl_getObjectCIDForCommand( std::string_view rDispatchCommand, const rtl::Reference<::chart::ChartModel> & xChartDocument, const OUString& rSelectedCID )
{
    ObjectType eObjectType = OBJECTTYPE_UNKNOWN;

    const ObjectType eSelectedType = ObjectIdentifier::getObjectType( rSelectedCID );
    rtl::Reference< DataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( rSelectedCID, xChartDocument );

    //legend
    if( rDispatchCommand == "Legend" || rDispatchCommand == "FormatLegend" )
    {
        eObjectType = OBJECTTYPE_LEGEND;
        //@todo set particular aParticleID if we have more than one legend
    }
    //wall floor area
    else if( rDispatchCommand == "DiagramWall" || rDispatchCommand == "FormatWall" )
    {
        //OBJECTTYPE_DIAGRAM;
        eObjectType = OBJECTTYPE_DIAGRAM_WALL;
        //@todo set particular aParticleID if we have more than one diagram
    }
    else if( rDispatchCommand == "DiagramFloor" || rDispatchCommand == "FormatFloor" )
    {
        eObjectType = OBJECTTYPE_DIAGRAM_FLOOR;
        //@todo set particular aParticleID if we have more than one diagram
    }
    else if( rDispatchCommand == "DiagramArea" || rDispatchCommand == "FormatChartArea" )
    {
        eObjectType = OBJECTTYPE_PAGE;
    }
    //title
    else if( rDispatchCommand == "MainTitle"
        || rDispatchCommand == "SubTitle"
        || rDispatchCommand == "XTitle"
        || rDispatchCommand == "YTitle"
        || rDispatchCommand == "ZTitle"
        || rDispatchCommand == "SecondaryXTitle"
        || rDispatchCommand == "SecondaryYTitle"
        || rDispatchCommand == "AllTitles"
        )
    {
        return lcl_getTitleCIDForCommand( rDispatchCommand, xChartDocument );
    }
    //axis
    else if( rDispatchCommand == "DiagramAxisX"
        || rDispatchCommand == "DiagramAxisY"
        || rDispatchCommand == "DiagramAxisZ"
        || rDispatchCommand == "DiagramAxisA"
        || rDispatchCommand == "DiagramAxisB"
        || rDispatchCommand == "DiagramAxisAll"
        )
    {
        return lcl_getAxisCIDForCommand( rDispatchCommand, xChartDocument );
    }
    //grid
    else if( rDispatchCommand == "DiagramGridYMain"
        || rDispatchCommand == "DiagramGridXMain"
        || rDispatchCommand == "DiagramGridZMain"
        || rDispatchCommand == "DiagramGridYHelp"
        || rDispatchCommand == "DiagramGridXHelp"
        || rDispatchCommand == "DiagramGridZHelp"
        || rDispatchCommand == "DiagramGridAll"
        )
    {
        return lcl_getGridCIDForCommand( rDispatchCommand, xChartDocument );
    }
    //data series
    else if( rDispatchCommand == "FormatDataSeries" )
    {
        if( eSelectedType == OBJECTTYPE_DATA_SERIES )
            return rSelectedCID;
        else
            return ObjectIdentifier::createClassifiedIdentifier(
                OBJECTTYPE_DATA_SERIES, ObjectIdentifier::getSeriesParticleFromCID( rSelectedCID ) );
    }
    //data point
    else if( rDispatchCommand == "FormatDataPoint" )
    {
        return rSelectedCID;
    }
    //data labels
    else if( rDispatchCommand == "FormatDataLabels" )
    {
        if( eSelectedType == OBJECTTYPE_DATA_LABELS )
            return rSelectedCID;
        else
            return ObjectIdentifier::createClassifiedIdentifierWithParent(
                OBJECTTYPE_DATA_LABELS, u"", rSelectedCID );
    }
    //data labels
    else if( rDispatchCommand == "FormatDataLabel" )
    {
        if( eSelectedType == OBJECTTYPE_DATA_LABEL )
            return rSelectedCID;
        else
        {
            sal_Int32 nPointIndex = o3tl::toInt32(ObjectIdentifier::getParticleID( rSelectedCID ));
            if( nPointIndex>=0 )
            {
                OUString aSeriesParticle = ObjectIdentifier::getSeriesParticleFromCID( rSelectedCID );
                OUString aChildParticle( ObjectIdentifier::getStringForType( OBJECTTYPE_DATA_LABELS ) + "=" );
                OUString aLabelsCID = ObjectIdentifier::createClassifiedIdentifierForParticles( aSeriesParticle, aChildParticle );
                OUString aLabelCID_Stub = ObjectIdentifier::createClassifiedIdentifierWithParent(
                    OBJECTTYPE_DATA_LABEL, u"", aLabelsCID );

                return ObjectIdentifier::createPointCID( aLabelCID_Stub, nPointIndex );
            }
        }
    }
    //mean value line
    else if( rDispatchCommand == "FormatMeanValue" )
    {
        if( eSelectedType == OBJECTTYPE_DATA_AVERAGE_LINE )
            return rSelectedCID;
        else
            return ObjectIdentifier::createDataCurveCID(
                ObjectIdentifier::getSeriesParticleFromCID( rSelectedCID ),
                    RegressionCurveHelper::getRegressionCurveIndex( xSeries,
                        RegressionCurveHelper::getMeanValueLine( xSeries ) ), true );
    }
    //trend line
    else if( rDispatchCommand == "FormatTrendline" )
    {
        if( eSelectedType == OBJECTTYPE_DATA_CURVE )
            return rSelectedCID;
        else
            return ObjectIdentifier::createDataCurveCID(
                ObjectIdentifier::getSeriesParticleFromCID( rSelectedCID ),
                    RegressionCurveHelper::getRegressionCurveIndex( xSeries,
                        RegressionCurveHelper::getFirstCurveNotMeanValueLine( xSeries ) ), false );
    }
    //trend line equation
    else if( rDispatchCommand == "FormatTrendlineEquation" )
    {
        if( eSelectedType == OBJECTTYPE_DATA_CURVE_EQUATION )
            return rSelectedCID;
        else
            return ObjectIdentifier::createDataCurveEquationCID(
                ObjectIdentifier::getSeriesParticleFromCID( rSelectedCID ),
                    RegressionCurveHelper::getRegressionCurveIndex( xSeries,
                        RegressionCurveHelper::getFirstCurveNotMeanValueLine( xSeries ) ) );
    }
    // y error bars
    else if( rDispatchCommand == "FormatXErrorBars" )
    {
        return lcl_getErrorCIDForCommand(OBJECTTYPE_DATA_ERRORS_X, eSelectedType, rSelectedCID );
    }
    // y error bars
    else if( rDispatchCommand == "FormatYErrorBars" )
    {
        return lcl_getErrorCIDForCommand(OBJECTTYPE_DATA_ERRORS_Y, eSelectedType, rSelectedCID );
    }
    // axis
    else if( rDispatchCommand == "FormatAxis" )
    {
        if( eSelectedType == OBJECTTYPE_AXIS )
            return rSelectedCID;
        else
        {
            rtl::Reference< Axis > xAxis = ObjectIdentifier::getAxisForCID( rSelectedCID, xChartDocument );
            return ObjectIdentifier::createClassifiedIdentifierForObject( xAxis , xChartDocument );
        }
    }
    // major grid
    else if( rDispatchCommand == "FormatMajorGrid" )
    {
        if( eSelectedType == OBJECTTYPE_GRID )
            return rSelectedCID;
        else
        {
            rtl::Reference< Axis > xAxis = ObjectIdentifier::getAxisForCID( rSelectedCID, xChartDocument );
            return ObjectIdentifier::createClassifiedIdentifierForGrid( xAxis, xChartDocument );
        }

    }
    // minor grid
    else if( rDispatchCommand == "FormatMinorGrid" )
    {
        if( eSelectedType == OBJECTTYPE_SUBGRID )
            return rSelectedCID;
        else
        {
            rtl::Reference< Axis > xAxis = ObjectIdentifier::getAxisForCID( rSelectedCID, xChartDocument );
            return ObjectIdentifier::createClassifiedIdentifierForGrid( xAxis, xChartDocument, 0 /*sub grid index*/ );
        }
    }
    // title
    else if( rDispatchCommand == "FormatTitle" )
    {
        if( eSelectedType == OBJECTTYPE_TITLE )
            return rSelectedCID;
    }
    // stock loss
    else if( rDispatchCommand == "FormatStockLoss" )
    {
        if( eSelectedType == OBJECTTYPE_DATA_STOCK_LOSS )
            return rSelectedCID;
        else
            return ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DATA_STOCK_LOSS, u"");
    }
    // stock gain
    else if( rDispatchCommand == "FormatStockGain" )
    {
        if( eSelectedType == OBJECTTYPE_DATA_STOCK_GAIN )
            return rSelectedCID;
        else
            return ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DATA_STOCK_GAIN, u"" );
    }

    return ObjectIdentifier::createClassifiedIdentifier(
        eObjectType,
        u"" ); // aParticleID
}

}
// anonymous namespace

void ChartController::executeDispatch_FormatObject(std::u16string_view rDispatchCommand)
{
    rtl::Reference<::chart::ChartModel> xChartDocument( getChartModel() );
    OString aCommand( OUStringToOString( rDispatchCommand, RTL_TEXTENCODING_ASCII_US ) );
    OUString rObjectCID = lcl_getObjectCIDForCommand( aCommand, xChartDocument, m_aSelection.getSelectedCID() );
    executeDlg_ObjectProperties( rObjectCID );
}

void ChartController::executeDispatch_ObjectProperties()
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
    if( eObjectType==OBJECTTYPE_LEGEND_ENTRY )
    {
        std::u16string_view aParentParticle( ObjectIdentifier::getFullParentParticle( rSelectedCID ) );
        aFormatCID  = ObjectIdentifier::createClassifiedIdentifierForParticle( aParentParticle );
    }

    // treat diagram as wall
    if( eObjectType==OBJECTTYPE_DIAGRAM )
        aFormatCID  = ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM_WALL, u"" );

    return aFormatCID;
}

}//end anonymous namespace

void ChartController::executeDlg_ObjectProperties( const OUString& rSelectedObjectCID )
{
    OUString aObjectCID = lcl_getFormatCIDforSelectedCID( rSelectedObjectCID );

    auto aUndoGuard = std::make_shared<UndoGuard>(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Format,
            ObjectNameProvider::getName( ObjectIdentifier::getObjectType( aObjectCID ))),
        m_xUndoManager );

    ChartController::executeDlg_ObjectProperties_withUndoGuard( aUndoGuard, aObjectCID, false );
}

void ChartController::executeDlg_ObjectProperties_withUndoGuard(
    std::shared_ptr<UndoGuard> aUndoGuard,const OUString& rObjectCID, bool bSuccessOnUnchanged )
{
    //return true if the properties were changed successfully
    if( rObjectCID.isEmpty() )
    {
       return;
    }
    try
    {
        //get type of object
        ObjectType eObjectType = ObjectIdentifier::getObjectType( rObjectCID );
        if( eObjectType==OBJECTTYPE_UNKNOWN )
        {
            return;
        }
        if( eObjectType==OBJECTTYPE_DIAGRAM_WALL || eObjectType==OBJECTTYPE_DIAGRAM_FLOOR )
        {
            if( !DiagramHelper::isSupportingFloorAndWall( getFirstDiagram() ) )
                return;
        }

        //convert properties to ItemSet

        std::unique_ptr<ReferenceSizeProvider> pRefSizeProv(impl_createReferenceSizeProvider());

        rtl::Reference<::chart::ChartModel> xChartDoc(getChartModel());

        std::shared_ptr<wrapper::ItemConverter> pItemConverter(
            createItemConverter( rObjectCID, xChartDoc, m_xCC,
                                 m_pDrawModelWrapper->getSdrModel(),
                                 comphelper::getFromUnoTunnel<ExplicitValueProvider>(m_xChartView),
                                 pRefSizeProv.get()));

        if (!pItemConverter)
            return;

        SfxItemSet aItemSet = pItemConverter->CreateEmptyItemSet();

        if ( eObjectType == OBJECTTYPE_DATA_ERRORS_X || eObjectType == OBJECTTYPE_DATA_ERRORS_Y )
            aItemSet.Put(SfxBoolItem(SCHATTR_STAT_ERRORBAR_TYPE, eObjectType == OBJECTTYPE_DATA_ERRORS_Y ));

        pItemConverter->FillItemSet(aItemSet);

        //prepare dialog
        ObjectPropertiesDialogParameter aDialogParameter( rObjectCID );
        aDialogParameter.init(xChartDoc);
        ViewElementListProvider aViewElementListProvider( m_pDrawModelWrapper.get() );

        SolarMutexGuard aGuard;
        std::shared_ptr<SchAttribTabDlg> aDlgPtr = std::make_shared<SchAttribTabDlg>(
            GetChartFrame(), &aItemSet, &aDialogParameter,
            &aViewElementListProvider,
            xChartDoc);

        if(aDialogParameter.HasSymbolProperties())
        {
            uno::Reference< beans::XPropertySet > xObjectProperties =
                ObjectIdentifier::getObjectPropertySet( rObjectCID, xChartDoc );
            wrapper::DataPointItemConverter aSymbolItemConverter( xChartDoc, m_xCC
                                        , xObjectProperties, ObjectIdentifier::getDataSeriesForCID( rObjectCID, xChartDoc )
                                        , m_pDrawModelWrapper->getSdrModel().GetItemPool()
                                        , m_pDrawModelWrapper->getSdrModel()
                                        , xChartDoc
                                        , wrapper::GraphicObjectType::FilledDataPoint );

            SfxItemSet aSymbolShapeProperties(aSymbolItemConverter.CreateEmptyItemSet() );
            aSymbolItemConverter.FillItemSet( aSymbolShapeProperties );

            sal_Int32 const nStandardSymbol=0;//@todo get from somewhere
            std::optional<Graphic> oAutoSymbolGraphic(std::in_place, aViewElementListProvider.GetSymbolGraphic( nStandardSymbol, &aSymbolShapeProperties ) );
            // note: the dialog takes the ownership of pSymbolShapeProperties and pAutoSymbolGraphic
            aDlgPtr->setSymbolInformation( std::move(aSymbolShapeProperties), std::move(oAutoSymbolGraphic) );
        }
        if( aDialogParameter.HasStatisticProperties() )
        {
            aDlgPtr->SetAxisMinorStepWidthForErrorBarDecimals(
                InsertErrorBarsDialog::getAxisMinorStepWidthForErrorBarDecimals( xChartDoc, m_xChartView, rObjectCID ) );
        }

        //open the dialog
        aDlgPtr->runAsync(aDlgPtr, [aDlgPtr, xChartDoc, pItemConverter, bSuccessOnUnchanged, aUndoGuard] (int nResult)
        {
            if (nResult == RET_OK || (bSuccessOnUnchanged && aDlgPtr->DialogWasClosedWithOK())) {
                const SfxItemSet* pOutItemSet = aDlgPtr->GetOutputItemSet();
                if(pOutItemSet) {
                    ControllerLockGuardUNO aCLGuard(xChartDoc);
                    (void)pItemConverter->ApplyItemSet(*pOutItemSet); //model should be changed now
                    aUndoGuard->commit();
                }
            }
        });
    }
    catch( const util::CloseVetoException& )
    {
    }
    catch( const uno::RuntimeException& )
    {
    }
}

void ChartController::executeDispatch_View3D()
{
    try
    {
        UndoLiveUpdateGuard aUndoGuard(
            SchResId( STR_ACTION_EDIT_3D_VIEW ),
            m_xUndoManager );

        //open dialog
        SolarMutexGuard aSolarGuard;
        View3DDialog aDlg(GetChartFrame(), getChartModel());
        if (aDlg.run() == RET_OK)
            aUndoGuard.commit();
    }
    catch(const uno::RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
