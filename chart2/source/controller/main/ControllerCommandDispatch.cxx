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

#include "ControllerCommandDispatch.hxx"
#include <ChartModel.hxx>
#include <Diagram.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <TitleHelper.hxx>
#include <LegendHelper.hxx>
#include <ObjectIdentifier.hxx>
#include <ChartType.hxx>
#include <ChartTypeHelper.hxx>
#include <ChartController.hxx>
#include <RegressionCurveHelper.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <StatisticsHelper.hxx>
#include <ReferenceSizeProvider.hxx>
#include "ShapeController.hxx"

#include <vcl/svapp.hxx>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>

#include <com/sun/star/chart2/XRegressionCurve.hpp>
#include <com/sun/star/chart2/XDataProviderAccess.hpp>

// only needed until #i68864# is fixed
#include <com/sun/star/frame/XLayoutManager.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{
bool lcl_isStatusBarVisible( const Reference< frame::XController > & xController )
{
    bool bIsStatusBarVisible = false;
    // Status-Bar visible, workaround: this should not be necessary. @todo:
    // remove when Issue #i68864# is fixed
    if( xController.is())
    {
        Reference< beans::XPropertySet > xPropSet( xController->getFrame(), uno::UNO_QUERY );
        if( xPropSet.is() )
        {
            uno::Reference< css::frame::XLayoutManager > xLayoutManager;
            xPropSet->getPropertyValue( u"LayoutManager"_ustr ) >>= xLayoutManager;
            if ( xLayoutManager.is() )
                bIsStatusBarVisible = xLayoutManager->isElementVisible( u"private:resource/statusbar/statusbar"_ustr );
        }
    }
    return bIsStatusBarVisible;
}

} // anonymous namespace

namespace chart
{

namespace impl
{

/// Constants for moving the series.
namespace {
    bool const MOVE_SERIES_FORWARD = true;
    bool const MOVE_SERIES_BACKWARD = false;
}

/** Represents the current state of the controller (needed for issue 63017).

    You can set the state by calling update().  After this call the state is
    preserved in this class until the next call to update().

    This is useful, not to say necessary, for enabling and disabling of menu
    entries (e.g. format>arrangement). As the status requests are sent very
    frequently it would be impossible, from a performance point of view, to
    query the current status every time directly at the model.  So this class
    serves as a cache for the state.
*/
struct ControllerState
{
    ControllerState();

    void update( const Reference< frame::XController > & xController,
                 const rtl::Reference<::chart::ChartModel> & xModel );

    // -- State variables -------
    bool bHasSelectedObject;
    bool bIsPositionableObject;
    bool bIsTextObject;
    bool bIsDeleteableObjectSelected;
    bool bIsFormateableObjectSelected;

    // May the selected series be moved forward or backward (cf
    // format>arrangement).
    bool bMayMoveSeriesForward;
    bool bMayMoveSeriesBackward;

    // trendlines
    bool bMayAddMenuTrendline;
    bool bMayAddTrendline;
    bool bMayAddTrendlineEquation;
    bool bMayAddR2Value;
    bool bMayAddMeanValue;
    bool bMayAddXErrorBars;
    bool bMayAddYErrorBars;

    bool bMayDeleteTrendline;
    bool bMayDeleteTrendlineEquation;
    bool bMayDeleteR2Value;
    bool bMayDeleteMeanValue;
    bool bMayDeleteXErrorBars;
    bool bMayDeleteYErrorBars;

    bool bMayFormatTrendline;
    bool bMayFormatTrendlineEquation;
    bool bMayFormatMeanValue;
    bool bMayFormatXErrorBars;
    bool bMayFormatYErrorBars;
};

ControllerState::ControllerState() :
        bHasSelectedObject( false ),
        bIsPositionableObject( false ),
        bIsTextObject(false),
        bIsDeleteableObjectSelected(false),
        bIsFormateableObjectSelected(false),
        bMayMoveSeriesForward( false ),
        bMayMoveSeriesBackward( false ),
        bMayAddMenuTrendline( false ),
        bMayAddTrendline( false ),
        bMayAddTrendlineEquation( false ),
        bMayAddR2Value( false ),
        bMayAddMeanValue( false ),
        bMayAddXErrorBars( false ),
        bMayAddYErrorBars( false ),
        bMayDeleteTrendline( false ),
        bMayDeleteTrendlineEquation( false ),
        bMayDeleteR2Value( false ),
        bMayDeleteMeanValue( false ),
        bMayDeleteXErrorBars( false ),
        bMayDeleteYErrorBars( false ),
        bMayFormatTrendline( false ),
        bMayFormatTrendlineEquation( false ),
        bMayFormatMeanValue( false ),
        bMayFormatXErrorBars( false ),
        bMayFormatYErrorBars( false )
{}

void ControllerState::update(
    const Reference< frame::XController > & xController,
    const rtl::Reference<::chart::ChartModel> & xModel )
{
    Reference< view::XSelectionSupplier > xSelectionSupplier(
        xController, uno::UNO_QUERY );

    // Update ControllerState variables.
    if( !xSelectionSupplier.is())
        return;

    uno::Any aSelObj( xSelectionSupplier->getSelection() );
    ObjectIdentifier aSelOID( aSelObj );
    OUString aSelObjCID( aSelOID.getObjectCID() );

    bHasSelectedObject = aSelOID.isValid();

    ObjectType aObjectType(ObjectIdentifier::getObjectType( aSelObjCID ));

    bIsPositionableObject = (aObjectType != OBJECTTYPE_DATA_POINT) && aSelOID.isDragableObject();
    bIsTextObject = aObjectType == OBJECTTYPE_TITLE;

    rtl::Reference< Diagram > xDiagram = xModel->getFirstChartDiagram();
    bIsFormateableObjectSelected = bHasSelectedObject && aSelOID.isAutoGeneratedObject();
    if( aObjectType==OBJECTTYPE_DIAGRAM || aObjectType==OBJECTTYPE_DIAGRAM_WALL || aObjectType==OBJECTTYPE_DIAGRAM_FLOOR )
        bIsFormateableObjectSelected = xDiagram->isSupportingFloorAndWall();

    rtl::Reference< DataSeries > xGivenDataSeries =
        ObjectIdentifier::getDataSeriesForCID(
            aSelObjCID, xModel );

    bIsDeleteableObjectSelected = ChartController::isObjectDeleteable( aSelObj );

    bMayMoveSeriesForward = (aObjectType!=OBJECTTYPE_DATA_POINT) && xDiagram && xDiagram->isSeriesMoveable(
        xGivenDataSeries,
        MOVE_SERIES_FORWARD );

    bMayMoveSeriesBackward = (aObjectType!=OBJECTTYPE_DATA_POINT) && xDiagram && xDiagram->isSeriesMoveable(
        xGivenDataSeries,
        MOVE_SERIES_BACKWARD );

    bMayAddMenuTrendline = false;
    bMayAddTrendline = false;
    bMayAddTrendlineEquation = false;
    bMayAddR2Value = false;
    bMayAddMeanValue = false;
    bMayAddXErrorBars = false;
    bMayAddYErrorBars = false;
    bMayDeleteTrendline = false;
    bMayDeleteTrendlineEquation = false;
    bMayDeleteR2Value = false;
    bMayDeleteMeanValue = false;
    bMayDeleteXErrorBars = false;
    bMayDeleteYErrorBars = false;
    bMayFormatTrendline = false;
    bMayFormatTrendlineEquation = false;
    bMayFormatMeanValue = false;
    bMayFormatXErrorBars = false;
    bMayFormatYErrorBars = false;
    if( !bHasSelectedObject )
        return;

    if( xGivenDataSeries.is())
    {
        bMayAddMenuTrendline = true;
        sal_Int32 nDimensionCount = xDiagram->getDimension();
        rtl::Reference< ::chart::ChartType > xFirstChartType(
            DataSeriesHelper::getChartTypeOfSeries( xGivenDataSeries, xDiagram ));

        // trend lines/mean value line
        if( (aObjectType == OBJECTTYPE_DATA_SERIES || aObjectType == OBJECTTYPE_DATA_POINT)
            && ChartTypeHelper::isSupportingRegressionProperties( xFirstChartType, nDimensionCount ))
        {
            // Trendline
            bMayAddTrendline = true;

            // Mean Value
            bMayFormatMeanValue = bMayDeleteMeanValue = RegressionCurveHelper::hasMeanValueLine( xGivenDataSeries );
            bMayAddMeanValue = ! bMayDeleteMeanValue;
        }

        // error bars
        if( (aObjectType == OBJECTTYPE_DATA_SERIES || aObjectType == OBJECTTYPE_DATA_POINT)
            && ChartTypeHelper::isSupportingStatisticProperties( xFirstChartType, nDimensionCount ))
        {
            bMayFormatXErrorBars = bMayDeleteXErrorBars = StatisticsHelper::hasErrorBars( xGivenDataSeries, false );
            bMayAddXErrorBars = ! bMayDeleteXErrorBars;

            bMayFormatYErrorBars = bMayDeleteYErrorBars = StatisticsHelper::hasErrorBars( xGivenDataSeries );
            bMayAddYErrorBars = ! bMayDeleteYErrorBars;
        }
    }

    if( aObjectType == OBJECTTYPE_DATA_AVERAGE_LINE )
        bMayFormatMeanValue = true;

    if( aObjectType == OBJECTTYPE_DATA_ERRORS_X)
        bMayFormatXErrorBars = true;

    if( aObjectType == OBJECTTYPE_DATA_ERRORS_Y )
        bMayFormatYErrorBars = true;

    if( aObjectType == OBJECTTYPE_DATA_CURVE )
    {
        bMayFormatTrendline = true;
        bMayDeleteTrendline = true;
        uno::Reference< chart2::XRegressionCurve > xRegCurve(
            ObjectIdentifier::getObjectPropertySet( aSelObjCID, xModel ), uno::UNO_QUERY );

        // Trendline Equation
        bMayFormatTrendlineEquation = bMayDeleteTrendlineEquation = RegressionCurveHelper::hasEquation( xRegCurve );
        bMayAddTrendlineEquation = !bMayDeleteTrendlineEquation;
        bMayAddR2Value = RegressionCurveHelper::MayHaveCorrelationCoefficient( xRegCurve ) && bMayAddTrendlineEquation;
    }
    else if( aObjectType == OBJECTTYPE_DATA_CURVE_EQUATION )
    {
        bMayFormatTrendlineEquation = true;
        bool bHasR2Value = false;
        bool bMayHaveR2 = true;
        try
        {
            uno::Reference< beans::XPropertySet > xEquationProperties =
                ObjectIdentifier::getObjectPropertySet( aSelObjCID, xModel );
            if( xEquationProperties.is() )
            {
                xEquationProperties->getPropertyValue( u"ShowCorrelationCoefficient"_ustr ) >>= bHasR2Value;
                xEquationProperties->getPropertyValue( u"MayHaveCorrelationCoefficient"_ustr ) >>= bMayHaveR2;
            }
        }
        catch(const uno::RuntimeException&)
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
        bMayAddR2Value = !bHasR2Value && bMayHaveR2;
        bMayDeleteR2Value = bHasR2Value;
    }
}

/** Represents the current state of the model.

    You can set the state by calling update().  After this call the state is
    preserved in this class until the next call to update().

    This is useful, not to say necessary, for enabling and disabling of menu
    entries and toolbar icons.  As the status requests are sent very frequently
    it would be impossible, from a performance point of view, to query the
    current status every time directly at the model.  So this class serves as a
    cache for the state.
 */
struct ModelState
{
    ModelState();

    void update( const rtl::Reference<::chart::ChartModel> & xModel );

    bool HasAnyAxis() const;
    bool HasAnyGrid() const;
    bool HasAnyTitle() const;

    bool bIsReadOnly;
    bool bIsThreeD;
    bool bHasOwnData;
    bool bHasDataFromPivotTable;

    bool bHasMainTitle;
    bool bHasSubTitle;
    bool bHasXAxisTitle;
    bool bHasYAxisTitle;
    bool bHasZAxisTitle;
    bool bHasSecondaryXAxisTitle;
    bool bHasSecondaryYAxisTitle;

    bool bHasXAxis;
    bool bHasYAxis;
    bool bHasZAxis;
    bool bHasAAxis;
    bool bHasBAxis;

    bool bHasMainXGrid;
    bool bHasMainYGrid;
    bool bHasMainZGrid;
    bool bHasHelpXGrid;
    bool bHasHelpYGrid;
    bool bHasHelpZGrid;

    bool bHasAutoScaledText;
    bool bHasLegend;
    bool bHasWall;
    bool bHasFloor;

    bool bSupportsStatistics;
    bool bSupportsAxes;

    bool bDataTable = false;
};

ModelState::ModelState() :
        bIsReadOnly(true),
        bIsThreeD(false),
        bHasOwnData(false),
        bHasDataFromPivotTable(false),
        bHasMainTitle(false),
        bHasSubTitle(false),
        bHasXAxisTitle(false),
        bHasYAxisTitle(false),
        bHasZAxisTitle(false),
        bHasSecondaryXAxisTitle(false),
        bHasSecondaryYAxisTitle(false),
        bHasXAxis(false),
        bHasYAxis(false),
        bHasZAxis(false),
        bHasAAxis(false),
        bHasBAxis(false),
        bHasMainXGrid(false),
        bHasMainYGrid(false),
        bHasMainZGrid(false),
        bHasHelpXGrid(false),
        bHasHelpYGrid(false),
        bHasHelpZGrid(false),
        bHasAutoScaledText(false),
        bHasLegend(false),
        bHasWall(false),
        bHasFloor(false),
        bSupportsStatistics(false),
        bSupportsAxes(false)
{}

void ModelState::update( const rtl::Reference<::chart::ChartModel> & xModel )
{
    rtl::Reference< Diagram > xDiagram = xModel->getFirstChartDiagram();

    bIsReadOnly = xModel->isReadonly();

    sal_Int32 nDimensionCount = 0;
    if (xDiagram)
        nDimensionCount = xDiagram->getDimension();

    rtl::Reference< ChartType > xFirstChartType;
    if (xDiagram)
        xFirstChartType = xDiagram->getChartTypeByIndex( 0 );
    bSupportsStatistics = ChartTypeHelper::isSupportingStatisticProperties( xFirstChartType, nDimensionCount );
    bSupportsAxes = ChartTypeHelper::isSupportingMainAxis( xFirstChartType, nDimensionCount, 0 );

    bIsThreeD = (nDimensionCount == 3);
    if (xModel.is())
    {
        bHasOwnData = xModel->hasInternalDataProvider();
        bHasDataFromPivotTable = !bHasOwnData && xModel->isDataFromPivotTable();
    }

    bHasMainTitle =  TitleHelper::getTitle( TitleHelper::MAIN_TITLE, xModel ).is();
    bHasSubTitle =   TitleHelper::getTitle( TitleHelper::SUB_TITLE, xModel ).is();
    bHasXAxisTitle = TitleHelper::getTitle( TitleHelper::X_AXIS_TITLE, xModel ).is();
    bHasYAxisTitle = TitleHelper::getTitle( TitleHelper::Y_AXIS_TITLE, xModel ).is();
    bHasZAxisTitle = TitleHelper::getTitle( TitleHelper::Z_AXIS_TITLE, xModel ).is();
    bHasSecondaryXAxisTitle = TitleHelper::getTitle( TitleHelper::SECONDARY_X_AXIS_TITLE, xModel ).is();
    bHasSecondaryYAxisTitle = TitleHelper::getTitle( TitleHelper::SECONDARY_Y_AXIS_TITLE, xModel ).is();

    bHasXAxis = bSupportsAxes && AxisHelper::getAxis( 0, true, xDiagram ).is();
    bHasYAxis = bSupportsAxes && AxisHelper::getAxis( 1, true, xDiagram ).is();
    bHasZAxis = bSupportsAxes && AxisHelper::getAxis( 2, true, xDiagram ).is();
    bHasAAxis = bSupportsAxes && AxisHelper::getAxis( 0, false, xDiagram ).is();
    bHasBAxis = bSupportsAxes && AxisHelper::getAxis( 1, false, xDiagram ).is();

    bHasMainXGrid = bSupportsAxes && AxisHelper::isGridShown( 0, 0, true, xDiagram );
    bHasMainYGrid = bSupportsAxes && AxisHelper::isGridShown( 1, 0, true, xDiagram );
    bHasMainZGrid = bSupportsAxes && AxisHelper::isGridShown( 2, 0, true, xDiagram );
    bHasHelpXGrid = bSupportsAxes && AxisHelper::isGridShown( 0, 0, false, xDiagram );
    bHasHelpYGrid = bSupportsAxes && AxisHelper::isGridShown( 1, 0, false, xDiagram );
    bHasHelpZGrid = bSupportsAxes && AxisHelper::isGridShown( 2, 0, false, xDiagram );

    bHasAutoScaledText =
        (ReferenceSizeProvider::getAutoResizeState( xModel ) ==
         ReferenceSizeProvider::AUTO_RESIZE_YES);

    bHasLegend = LegendHelper::hasLegend( xDiagram );
    bHasWall = xDiagram && xDiagram->isSupportingFloorAndWall();
    bHasFloor = bHasWall && bIsThreeD;

    bDataTable = xDiagram.is() && xDiagram->getDataTable().is();
}

bool ModelState::HasAnyAxis() const
{
    return bHasXAxis || bHasYAxis || bHasZAxis || bHasAAxis || bHasBAxis;
}

bool ModelState::HasAnyGrid() const
{
    return bHasMainXGrid || bHasMainYGrid || bHasMainZGrid ||
        bHasHelpXGrid || bHasHelpYGrid || bHasHelpZGrid;
}

bool ModelState::HasAnyTitle() const
{
    return bHasMainTitle || bHasSubTitle || bHasXAxisTitle || bHasYAxisTitle || bHasZAxisTitle || bHasSecondaryXAxisTitle || bHasSecondaryYAxisTitle;
}

} // namespace impl

ControllerCommandDispatch::ControllerCommandDispatch(
    const Reference< uno::XComponentContext > & xContext,
    ChartController* pController, CommandDispatchContainer* pContainer ) :
        impl::ControllerCommandDispatch_Base( xContext ),
        m_xChartController( pController ),
        m_xSelectionSupplier( Reference< view::XSelectionSupplier >( pController ) ),
        m_xDispatch( Reference< frame::XDispatch >( pController ) ),
        m_apModelState( new impl::ModelState() ),
        m_apControllerState( new impl::ControllerState() ),
        m_pDispatchContainer( pContainer )
{
}

ControllerCommandDispatch::~ControllerCommandDispatch()
{
}

void ControllerCommandDispatch::initialize()
{
    if( !m_xChartController.is())
        return;

    rtl::Reference<::chart::ChartModel> xModel( m_xChartController->getChartModel());
    OSL_ASSERT( xModel.is());
    if( xModel.is())
        xModel->addModifyListener( this );

    // Listen selection modifications (Arrangement feature - issue 63017).
    if( m_xSelectionSupplier.is() )
        m_xSelectionSupplier->addSelectionChangeListener( this );

    if( m_apModelState && xModel.is())
        m_apModelState->update( xModel );

    if( m_apControllerState && xModel.is())
        m_apControllerState->update( m_xChartController, xModel );

    updateCommandAvailability();
}

void ControllerCommandDispatch::fireStatusEventForURLImpl(
    const OUString & rURL,
    const Reference< frame::XStatusListener > & xSingleListener )
{
    std::map< OUString, uno::Any >::const_iterator aArgIt( m_aCommandArguments.find( rURL ));
    if( aArgIt != m_aCommandArguments.end())
        fireStatusEventForURL( rURL, aArgIt->second, commandAvailable( rURL ), xSingleListener );
    else
        fireStatusEventForURL( rURL, uno::Any(), commandAvailable( rURL ), xSingleListener );
}

void ControllerCommandDispatch::updateCommandAvailability()
{
    bool bModelStateIsValid = (m_apModelState != nullptr);
    bool bControllerStateIsValid = (m_apControllerState != nullptr);
    // Model and controller states exist.
    OSL_ASSERT( bModelStateIsValid );
    OSL_ASSERT( bControllerStateIsValid );

    // read-only
    bool bIsWritable = bModelStateIsValid && (! m_apModelState->bIsReadOnly);
    bool bShapeContext = m_xChartController.is() && m_xChartController->isShapeContext();
    bool bIsTextEdit = m_xChartController.is() && m_xChartController->IsTextEdit();

    bool bEnableDataTableDialog = false;
    bool bCanCreateDataProvider = false;

    if ( m_xChartController.is() )
    {
        Reference< beans::XPropertySet > xProps( m_xChartController->getModel(), uno::UNO_QUERY );
        if ( xProps.is() )
        {
            try
            {
                xProps->getPropertyValue(u"EnableDataTableDialog"_ustr) >>= bEnableDataTableDialog;
            }
            catch( const uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION("chart2", "" );
            }
        }

        rtl::Reference< ChartModel > xChartModel = m_xChartController->getChartModel();
        OSL_ENSURE(xChartModel.is(), "Invalid XChartDocument");
        if ( xChartModel.is() )
        {
            css::uno::Reference< com::sun::star::chart2::XDataProviderAccess > xCreatorDoc(xChartModel->getParent(), uno::UNO_QUERY);
            bCanCreateDataProvider = xCreatorDoc.is();
        }
    }

    // edit commands
    m_aCommandAvailability[ u".uno:Cut"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bIsDeleteableObjectSelected;
    m_aCommandAvailability[ u".uno:Copy"_ustr ] = bControllerStateIsValid && m_apControllerState->bHasSelectedObject;
    m_aCommandAvailability[ u".uno:Paste"_ustr ] = bIsWritable;

    // toolbar commands
    m_aCommandAvailability[ u".uno:ToggleGridHorizontal"_ustr ] = bIsWritable;
    m_aCommandArguments[ u".uno:ToggleGridHorizontal"_ustr ] <<= bModelStateIsValid && m_apModelState->bHasMainYGrid;
    m_aCommandAvailability[ u".uno:ToggleGridVertical"_ustr ] = bIsWritable;
    m_aCommandArguments[ u".uno:ToggleGridVertical"_ustr ] <<= bModelStateIsValid && m_apModelState->bHasMainXGrid;

    m_aCommandAvailability[ u".uno:ToggleLegend"_ustr ] = bIsWritable;
    m_aCommandArguments[ u".uno:ToggleLegend"_ustr ] <<= bModelStateIsValid && m_apModelState->bHasLegend;

    m_aCommandAvailability[ u".uno:NewArrangement"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:Update"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:DefaultColors"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:BarWidth"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:NumberOfLines"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:ArrangeRow"_ustr ] =
        bShapeContext || ( bIsWritable && bControllerStateIsValid && ( m_apControllerState->bMayMoveSeriesForward || m_apControllerState->bMayMoveSeriesBackward ) );

    // insert objects
    m_aCommandAvailability[ u".uno:InsertTitles"_ustr ] = m_aCommandAvailability[ u".uno:InsertMenuTitles"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:InsertLegend"_ustr ] = m_aCommandAvailability[ u".uno:InsertMenuLegend"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:DeleteLegend"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:InsertMenuDataLabels"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:InsertRemoveAxes"_ustr ] = m_aCommandAvailability[ u".uno:InsertMenuAxes"_ustr ] = bIsWritable && m_apModelState->bSupportsAxes;
    m_aCommandAvailability[ u".uno:InsertMenuGrids"_ustr ] = bIsWritable && m_apModelState->bSupportsAxes;
    m_aCommandAvailability[ u".uno:InsertMenuTrendlines"_ustr ] = bIsWritable && m_apModelState->bSupportsStatistics && bControllerStateIsValid && m_apControllerState->bMayAddMenuTrendline;
    m_aCommandAvailability[ u".uno:InsertMenuMeanValues"_ustr ] = bIsWritable && m_apModelState->bSupportsStatistics;
    m_aCommandAvailability[ u".uno:InsertMenuXErrorBars"_ustr ] = bIsWritable && m_apModelState->bSupportsStatistics;
    m_aCommandAvailability[ u".uno:InsertMenuYErrorBars"_ustr ] = bIsWritable && m_apModelState->bSupportsStatistics;
    m_aCommandAvailability[ u".uno:InsertSymbol"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bIsTextObject;
    m_aCommandAvailability[ u".uno:InsertMenuDataTable"_ustr ] = bIsWritable;

    // format objects
    bool bFormatObjectAvailable = bIsWritable && bControllerStateIsValid && m_apControllerState->bIsFormateableObjectSelected;
    m_aCommandAvailability[ u".uno:FormatSelection"_ustr ] = bFormatObjectAvailable && !bIsTextEdit;
    m_aCommandAvailability[ u".uno:FontDialog"_ustr ] = bFormatObjectAvailable && bIsTextEdit;
    m_aCommandAvailability[ u".uno:FormatAxis"_ustr ] = bFormatObjectAvailable;
    m_aCommandAvailability[ u".uno:FormatTitle"_ustr ] = bFormatObjectAvailable && !bIsTextEdit;
    m_aCommandAvailability[ u".uno:FormatDataSeries"_ustr ] = bFormatObjectAvailable;
    m_aCommandAvailability[ u".uno:FormatDataPoint"_ustr ] = bFormatObjectAvailable;
    m_aCommandAvailability[ u".uno:FormatDataLabels"_ustr ] = bFormatObjectAvailable;
    m_aCommandAvailability[ u".uno:FormatDataLabel"_ustr ] = bFormatObjectAvailable;
    m_aCommandAvailability[ u".uno:FormatXErrorBars"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayFormatXErrorBars;
    m_aCommandAvailability[ u".uno:FormatYErrorBars"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayFormatYErrorBars;
    m_aCommandAvailability[ u".uno:FormatMeanValue"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayFormatMeanValue;
    m_aCommandAvailability[ u".uno:FormatTrendline"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayFormatTrendline;
    m_aCommandAvailability[ u".uno:FormatTrendlineEquation"_ustr ] = bFormatObjectAvailable && bControllerStateIsValid && m_apControllerState->bMayFormatTrendlineEquation;
    m_aCommandAvailability[ u".uno:FormatStockLoss"_ustr ] = bFormatObjectAvailable;
    m_aCommandAvailability[ u".uno:FormatStockGain"_ustr ] = bFormatObjectAvailable;

    m_aCommandAvailability[ u".uno:DiagramType"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:Legend"_ustr ] = bIsWritable && m_apModelState->bHasLegend;
    m_aCommandAvailability[ u".uno:DiagramWall"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasWall;
    m_aCommandAvailability[ u".uno:DiagramArea"_ustr ] = bIsWritable;

    m_aCommandAvailability[ u".uno:TransformDialog"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bHasSelectedObject && m_apControllerState->bIsPositionableObject;

    // 3d commands
    m_aCommandAvailability[ u".uno:View3D"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bIsThreeD;
    m_aCommandAvailability[ u".uno:DiagramFloor"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasFloor;

    //some more format commands with different ui text
    m_aCommandAvailability[ u".uno:FormatWall"_ustr ] = m_aCommandAvailability[ u".uno:DiagramWall"_ustr ];
    m_aCommandAvailability[ u".uno:FormatFloor"_ustr ] = m_aCommandAvailability[ u".uno:DiagramFloor"_ustr ];
    m_aCommandAvailability[ u".uno:FormatChartArea"_ustr ] = m_aCommandAvailability[ u".uno:DiagramArea"_ustr ];
    m_aCommandAvailability[ u".uno:FormatLegend"_ustr ] = m_aCommandAvailability[ u".uno:Legend"_ustr ];

    // depending on own data and ability to create new data provider
    m_aCommandAvailability[u".uno:DataRanges"_ustr] = bIsWritable && bModelStateIsValid && !m_apModelState->bHasDataFromPivotTable
        && ((m_apModelState->bHasOwnData && bCanCreateDataProvider) || !m_apModelState->bHasOwnData);
    m_aCommandAvailability[ u".uno:DiagramData"_ustr ] = bIsWritable && bModelStateIsValid &&  m_apModelState->bHasOwnData && bEnableDataTableDialog;

    // titles
    m_aCommandAvailability[ u".uno:MainTitle"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasMainTitle;
    m_aCommandAvailability[ u".uno:SubTitle"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasSubTitle;
    m_aCommandAvailability[ u".uno:XTitle"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasXAxisTitle;
    m_aCommandAvailability[ u".uno:YTitle"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasYAxisTitle;
    m_aCommandAvailability[ u".uno:ZTitle"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasZAxisTitle;
    m_aCommandAvailability[ u".uno:SecondaryXTitle"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasSecondaryXAxisTitle;
    m_aCommandAvailability[ u".uno:SecondaryYTitle"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasSecondaryYAxisTitle;
    m_aCommandAvailability[ u".uno:AllTitles"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->HasAnyTitle();

    // text
    m_aCommandAvailability[ u".uno:ScaleText"_ustr ] = bIsWritable && bModelStateIsValid ;
    m_aCommandArguments[ u".uno:ScaleText"_ustr ] <<= bModelStateIsValid && m_apModelState->bHasAutoScaledText;

    // axes
    m_aCommandAvailability[ u".uno:DiagramAxisX"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasXAxis;
    m_aCommandAvailability[ u".uno:DiagramAxisY"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasYAxis;
    m_aCommandAvailability[ u".uno:DiagramAxisZ"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasZAxis;
    m_aCommandAvailability[ u".uno:DiagramAxisA"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasAAxis;
    m_aCommandAvailability[ u".uno:DiagramAxisB"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasBAxis;
    m_aCommandAvailability[ u".uno:DiagramAxisAll"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->HasAnyAxis();

    // grids
    // note: x and y are swapped in the commands!
    m_aCommandAvailability[ u".uno:DiagramGridYMain"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasMainXGrid;
    m_aCommandAvailability[ u".uno:DiagramGridXMain"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasMainYGrid;
    m_aCommandAvailability[ u".uno:DiagramGridZMain"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasMainZGrid;
    m_aCommandAvailability[ u".uno:DiagramGridYHelp"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasHelpXGrid;
    m_aCommandAvailability[ u".uno:DiagramGridXHelp"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasHelpYGrid;
    m_aCommandAvailability[ u".uno:DiagramGridZHelp"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bHasHelpZGrid;
    m_aCommandAvailability[ u".uno:DiagramGridAll"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->HasAnyGrid();

    // series arrangement
    m_aCommandAvailability[ u".uno:Forward"_ustr ] = ( bShapeContext ? isShapeControllerCommandAvailable( u".uno:Forward"_ustr ) :
        ( bIsWritable && bControllerStateIsValid && m_apControllerState->bMayMoveSeriesForward && bEnableDataTableDialog ) );
    m_aCommandAvailability[ u".uno:Backward"_ustr ] = ( bShapeContext ? isShapeControllerCommandAvailable( u".uno:Backward"_ustr ) :
        ( bIsWritable && bControllerStateIsValid && m_apControllerState->bMayMoveSeriesBackward && bEnableDataTableDialog ) );

    m_aCommandAvailability[ u".uno:InsertDataLabels"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:InsertDataLabel"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:InsertMeanValue"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayAddMeanValue;
    m_aCommandAvailability[ u".uno:InsertTrendline"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayAddTrendline;
    const bool bInsertTrendlineEquation = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayAddTrendlineEquation;
    m_aCommandAvailability[ u".uno:InsertTrendlineEquation"_ustr ] = bInsertTrendlineEquation;
    m_aCommandAvailability[ u".uno:InsertTrendlineEquationAndR2"_ustr ] = bInsertTrendlineEquation && m_apControllerState->bMayAddR2Value;
    m_aCommandAvailability[ u".uno:InsertR2Value"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayAddR2Value
        && !m_apControllerState->bMayAddTrendlineEquation;
    m_aCommandAvailability[ u".uno:DeleteR2Value"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayDeleteR2Value;

    m_aCommandAvailability[ u".uno:InsertXErrorBars"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayAddXErrorBars;
    m_aCommandAvailability[ u".uno:InsertYErrorBars"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayAddYErrorBars;

    m_aCommandAvailability[ u".uno:DeleteDataLabels"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:DeleteDataLabel"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:DeleteTrendline"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayDeleteTrendline;
    m_aCommandAvailability[ u".uno:DeleteTrendlineEquation"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayDeleteTrendlineEquation;
    m_aCommandAvailability[ u".uno:DeleteMeanValue"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayDeleteMeanValue;
    m_aCommandAvailability[ u".uno:DeleteXErrorBars"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayDeleteXErrorBars;
    m_aCommandAvailability[ u".uno:DeleteYErrorBars"_ustr ] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayDeleteYErrorBars;

    m_aCommandAvailability[ u".uno:ResetDataPoint"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:ResetAllDataPoints"_ustr ] = bIsWritable;

    m_aCommandAvailability[ u".uno:InsertAxis"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:DeleteAxis"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:InsertAxisTitle"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:FormatMajorGrid"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:InsertMajorGrid"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:DeleteMajorGrid"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:FormatMinorGrid"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:InsertMinorGrid"_ustr ] = bIsWritable;
    m_aCommandAvailability[ u".uno:DeleteMinorGrid"_ustr ] = bIsWritable;

    // data table
    m_aCommandAvailability[ u".uno:InsertDataTable"_ustr ] = bIsWritable && bModelStateIsValid && !m_apModelState->bDataTable;
    m_aCommandAvailability[ u".uno:DeleteDataTable"_ustr ] = bIsWritable && bModelStateIsValid && m_apModelState->bDataTable;
}

bool ControllerCommandDispatch::commandAvailable( const OUString & rCommand )
{
    std::map< OUString, bool >::const_iterator aIt( m_aCommandAvailability.find( rCommand ));
    if( aIt != m_aCommandAvailability.end())
        return aIt->second;
    SAL_WARN("chart2", "commandAvailable: command not in availability map:" << rCommand);
    return false;
}

bool ControllerCommandDispatch::isShapeControllerCommandAvailable( const OUString& rCommand )
{
    ShapeController* pShapeController(nullptr);
    {
        SolarMutexGuard g;
        if (m_pDispatchContainer)
            pShapeController = m_pDispatchContainer->getShapeController();
    }
    if ( pShapeController )
    {
        FeatureState aState( pShapeController->getState( rCommand ) );
        return aState.bEnabled;
    }
    return false;
}

void ControllerCommandDispatch::fireStatusEvent(
    const OUString & rURL,
    const Reference< frame::XStatusListener > & xSingleListener /* = 0 */ )
{
    bool bIsChartSelectorURL = rURL == ".uno:ChartElementSelector";
    if( rURL.isEmpty() || bIsChartSelectorURL )
    {
        uno::Any aArg;
        aArg <<= Reference< frame::XController >(m_xChartController);
        fireStatusEventForURL( u".uno:ChartElementSelector"_ustr, aArg, true, xSingleListener );
    }

    if( rURL.isEmpty() )
    {
        for (auto const& elem : m_aCommandAvailability)
            fireStatusEventForURLImpl( elem.first, xSingleListener );
    }
    else if( !bIsChartSelectorURL )
        fireStatusEventForURLImpl( rURL, xSingleListener );

    // statusbar. Should be handled by base implementation
    // @todo: remove if Issue 68864 is fixed
    if( rURL.isEmpty() || rURL == ".uno:StatusBarVisible" )
    {
        bool bIsStatusBarVisible( lcl_isStatusBarVisible( m_xChartController ));
        fireStatusEventForURL( u".uno:StatusBarVisible"_ustr, uno::Any( bIsStatusBarVisible ), true, xSingleListener );
    }
}

// ____ XDispatch ____
void SAL_CALL ControllerCommandDispatch::dispatch(
    const util::URL& URL,
    const Sequence< beans::PropertyValue >& Arguments )
{
    if( commandAvailable( URL.Complete ))
        m_xDispatch->dispatch( URL, Arguments );
}

// ____ WeakComponentImplHelperBase ____
/// is called when this is disposed
void ControllerCommandDispatch::disposing(std::unique_lock<std::mutex>& /*rGuard*/)
{
    m_xChartController.clear();
    m_xDispatch.clear();
    m_xSelectionSupplier.clear();
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL ControllerCommandDispatch::disposing( const lang::EventObject& /* Source */ )
{
    m_xChartController.clear();
    m_xDispatch.clear();
    m_xSelectionSupplier.clear();
}

// ____ XModifyListener ____
void SAL_CALL ControllerCommandDispatch::modified( const lang::EventObject& aEvent )
{
    bool bUpdateCommandAvailability = false;

    // Update the "ModelState" Struct.
    if( m_apModelState && m_xChartController.is())
    {
        m_apModelState->update( m_xChartController->getChartModel());
        bUpdateCommandAvailability = true;
    }

    // Update the "ControllerState" Struct.
    if( m_apControllerState && m_xChartController.is())
    {
        m_apControllerState->update( m_xChartController, m_xChartController->getChartModel());
        bUpdateCommandAvailability = true;
    }

    if( bUpdateCommandAvailability )
        updateCommandAvailability();

    if (comphelper::LibreOfficeKit::isActive())
    {
        if (SfxViewShell* pViewShell = SfxViewShell::Current())
            if (SfxObjectShell* pObjSh = pViewShell->GetObjectShell())
                pObjSh->SetModified();
    }

    CommandDispatch::modified( aEvent );
}

// ____ XSelectionChangeListener ____
void SAL_CALL ControllerCommandDispatch::selectionChanged( const lang::EventObject& aEvent )
{
    // Update the "ControllerState" Struct.
    if( m_apControllerState && m_xChartController.is())
    {
        m_apControllerState->update( m_xChartController, m_xChartController->getChartModel());
        updateCommandAvailability();
    }

    CommandDispatch::modified( aEvent );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
