/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ControllerCommandDispatch.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:58:43 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "ControllerCommandDispatch.hxx"
#include "ChartModelHelper.hxx"
#include "DiagramHelper.hxx"
#include "AxisHelper.hxx"
#include "TitleHelper.hxx"
#include "LegendHelper.hxx"
#include "ObjectIdentifier.hxx"
#include "macros.hxx"
#include "ChartTypeHelper.hxx"
#include "DiagramHelper.hxx"
#include "ChartController.hxx"
#include "RegressionCurveHelper.hxx"
#include "DataSeriesHelper.hxx"

#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XRegressionCurve.hpp>

// only needed until #i68864# is fixed
#ifndef _COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <com/sun/star/frame/XLayoutManager.hpp>
#endif

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

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
            uno::Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
            xPropSet->getPropertyValue( C2U( "LayoutManager" ) ) >>= xLayoutManager;
            if ( xLayoutManager.is() )
                bIsStatusBarVisible = xLayoutManager->isElementVisible( C2U("private:resource/statusbar/statusbar"));
        }
    }
    return bIsStatusBarVisible;
}

} // anonymous namespace

namespace chart
{

// ----------------------------------------

namespace impl
{

/// Constants for moving the series.
enum EnumForward{
    MOVE_SERIES_FORWARD = true,
    MOVE_SERIES_BACKWARD = false
};

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
                 const Reference< frame::XModel > & xModel );

    // -- State variables -------
    bool bHasSelectedObject;
    bool bIsDraggableObject;
    bool bIsTextObject;
    bool bIsDeleteableObjectSelected;
    bool bIsFormateableObjectSelected;

    // May the selected series be moved forward or backward (cf
    // format>arrangement).
    bool bMayMoveSeriesForward;
    bool bMayMoveSeriesBackward;

    // trendlines
    bool bMayAddTrendline;
    bool bMayAddTrendlineEquation;
    bool bMayAddMeanValue;
};


ControllerState::ControllerState() :
        bHasSelectedObject( false ),
        bIsDraggableObject( false ),
        bIsTextObject(false),
        bIsDeleteableObjectSelected(false),
        bIsFormateableObjectSelected(false),
        bMayMoveSeriesForward( false ),
        bMayMoveSeriesBackward( false ),
        bMayAddTrendline( false ),
        bMayAddTrendlineEquation( false ),
        bMayAddMeanValue( false )
{}

void ControllerState::update(
    const Reference< frame::XController > & xController,
    const Reference< frame::XModel > & xModel )
{
    Reference< view::XSelectionSupplier > xSelectionSupplier(
        xController, uno::UNO_QUERY );

    OUString aSelObjCID;

    // Update ControllerState variables.
    if( xSelectionSupplier.is())
    {
        uno::Any aSelObj( xSelectionSupplier->getSelection() );

        bHasSelectedObject = ((aSelObj >>= aSelObjCID) && aSelObjCID.getLength() > 0);

        bIsDraggableObject = ObjectIdentifier::isDragableObject( aSelObjCID );

        ObjectType aObjectType(ObjectIdentifier::getObjectType( aSelObjCID ));
        bIsTextObject = OBJECTTYPE_TITLE == aObjectType;

        uno::Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram( xModel ));
        bIsFormateableObjectSelected = bHasSelectedObject;
        if( OBJECTTYPE_DIAGRAM==aObjectType || OBJECTTYPE_DIAGRAM_WALL==aObjectType || OBJECTTYPE_DIAGRAM_FLOOR==aObjectType )
            bIsFormateableObjectSelected = DiagramHelper::isSupportingFloorAndWall( xDiagram );

        uno::Reference< chart2::XDataSeries > xGivenDataSeries(
            ObjectIdentifier::getDataSeriesForCID(
                aSelObjCID, xModel ) );

        bIsDeleteableObjectSelected = ChartController::isObjectDeleteable( aSelObj );

        bMayMoveSeriesForward = DiagramHelper::isSeriesMoveable(
            ChartModelHelper::findDiagram( xModel ),
            xGivenDataSeries,
            MOVE_SERIES_FORWARD );

        bMayMoveSeriesBackward = DiagramHelper::isSeriesMoveable(
            ChartModelHelper::findDiagram( xModel ),
            xGivenDataSeries,
            MOVE_SERIES_BACKWARD );

        bMayAddTrendline = false;
        bMayAddTrendlineEquation = false;
        bMayAddMeanValue = false;
        if( bHasSelectedObject )
        {
            if( xGivenDataSeries.is())
            {
                sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );
                if( ChartTypeHelper::isSupportingRegressionProperties(
                    DataSeriesHelper::getChartTypeOfSeries( xGivenDataSeries, xDiagram ),
                    nDimensionCount ))
                {
                    uno::Reference< chart2::XRegressionCurveContainer > xRegCurveCnt(
                        xGivenDataSeries, uno::UNO_QUERY );
                    if( xRegCurveCnt.is())
                    {
                        bMayAddTrendline = ! RegressionCurveHelper::getFirstCurveNotMeanValueLine( xRegCurveCnt ).is();
                        bMayAddMeanValue = ! RegressionCurveHelper::hasMeanValueLine( xRegCurveCnt );
                    }
                }
            }

            if( aObjectType == OBJECTTYPE_DATA_CURVE )
            {
                uno::Reference< chart2::XRegressionCurve > xRegCurve(
                    ObjectIdentifier::getObjectPropertySet( aSelObjCID, xModel ), uno::UNO_QUERY );
                if( xRegCurve.is())
                {
                    uno::Reference< beans::XPropertySet > xEqProp( xRegCurve->getEquationProperties());
                    bool bShowEq = false;
                    bool bShowCorr = false;
                    if( xEqProp.is())
                    {
                        xEqProp->getPropertyValue( C2U("ShowEquation")) >>= bShowEq;
                        xEqProp->getPropertyValue( C2U("ShowCorrelationCoefficient")) >>= bShowCorr;

                        bMayAddTrendlineEquation = ! (bShowEq || bShowCorr);
                    }
                }
            }
        }
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

    void update( const Reference< frame::XModel > & xModel );

    bool HasAnyAxis() const;
    bool HasAnyGrid() const;
    bool HasAnyTitle() const;

    bool bIsReadOnly;
    bool bIsThreeD;
    bool bHasOwnData;

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
};

ModelState::ModelState() :
        bIsReadOnly( true ),
        bIsThreeD( false ),
        bHasOwnData( false ),
        bHasMainTitle( false ),
        bHasSubTitle( false ),
        bHasXAxisTitle( false ),
        bHasYAxisTitle( false ),
        bHasZAxisTitle( false ),
        bHasSecondaryXAxisTitle( false ),
        bHasSecondaryYAxisTitle( false ),
        bHasXAxis( false ),
        bHasYAxis( false ),
        bHasZAxis( false ),
        bHasAAxis( false ),
        bHasBAxis( false ),
        bHasMainXGrid( false ),
        bHasMainYGrid( false ),
        bHasMainZGrid( false ),
        bHasHelpXGrid( false ),
        bHasHelpYGrid( false ),
        bHasHelpZGrid( false ),
        bHasAutoScaledText( false ),
        bHasLegend( false ),
        bHasWall( false ),
        bHasFloor( false ),
        bSupportsStatistics( false ),
        bSupportsAxes( false )

{}

void ModelState::update( const Reference< frame::XModel > & xModel )
{
    Reference< chart2::XChartDocument > xChartDoc( xModel, uno::UNO_QUERY );
    Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram( xModel ));

    bIsReadOnly = true;
    Reference< frame::XStorable > xStorable( xModel, uno::UNO_QUERY );
    if( xStorable.is())
        bIsReadOnly = xStorable->isReadonly();

    sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );

    uno::Reference< chart2::XChartType > xFirstChartType( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );
    bSupportsStatistics = ChartTypeHelper::isSupportingStatisticProperties( xFirstChartType, nDimensionCount );
    bSupportsAxes = ChartTypeHelper::isSupportingMainAxis( xFirstChartType, nDimensionCount, 0 );

    bIsThreeD = (nDimensionCount == 3);
    bHasOwnData = (xChartDoc.is() && xChartDoc->hasInternalDataProvider());

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
        (ReferenceSizeProvider::getAutoResizeState( xChartDoc ) ==
         ReferenceSizeProvider::AUTO_RESIZE_YES);

    bHasLegend = LegendHelper::hasLegend( xDiagram );
    bHasWall = DiagramHelper::isSupportingFloorAndWall( xDiagram );
    bHasFloor = bHasWall && bIsThreeD;
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

// ----------------------------------------

ControllerCommandDispatch::ControllerCommandDispatch(
    const Reference< uno::XComponentContext > & xContext,
    const Reference< frame::XController > & xController ) :
        impl::ControllerCommandDispatch_Base( xContext ),
        m_xController( xController ),
        m_xSelectionSupplier( xController, uno::UNO_QUERY ),
        m_xDispatch( xController, uno::UNO_QUERY ),
        m_apModelState( new impl::ModelState() ),
                m_apControllerState( new impl::ControllerState() )
{}

ControllerCommandDispatch::~ControllerCommandDispatch()
{}

void ControllerCommandDispatch::initialize()
{
    if( m_xController.is())
    {
        Reference< frame::XModel > xModel( m_xController->getModel());
        Reference< util::XModifyBroadcaster > xModifyBroadcaster( xModel, uno::UNO_QUERY );
        OSL_ASSERT( xModifyBroadcaster.is());
        if( xModifyBroadcaster.is())
            xModifyBroadcaster->addModifyListener( this );

                // Listen selection modifications (Arrangement feature - issue 63017).
                if( m_xSelectionSupplier.is() )
                        m_xSelectionSupplier->addSelectionChangeListener( this );

        if( m_apModelState.get() && xModel.is())
            m_apModelState->update( xModel );

        if( m_apControllerState.get() && xModel.is())
            m_apControllerState->update( m_xController, xModel );

        updateCommandAvailability();
    }
}

void ControllerCommandDispatch::fireStatusEventForURLImpl(
    const OUString & rURL,
    const Reference< frame::XStatusListener > & xSingleListener )
{
    ::std::map< OUString, uno::Any >::const_iterator aArgIt( m_aCommandArguments.find( rURL ));
    if( aArgIt != m_aCommandArguments.end())
        fireStatusEventForURL( rURL, aArgIt->second, commandAvailable( rURL ), xSingleListener );
    else
        fireStatusEventForURL( rURL, uno::Any(), commandAvailable( rURL ), xSingleListener );
}

void ControllerCommandDispatch::updateCommandAvailability()
{
    bool bModelStateIsValid = ( m_apModelState.get() != 0 );
    bool bControllerStateIsValid = ( m_apControllerState.get() != 0 );
    // Model and controller states exist.
    OSL_ASSERT( bModelStateIsValid );
    OSL_ASSERT( bControllerStateIsValid );

    // read-only
    bool bIsWritable = bModelStateIsValid && (! m_apModelState->bIsReadOnly);
    // paste is available
    // @todo: determine correctly
    bool bHasSuitableClipboardContent = true;

    // edit commands
    m_aCommandAvailability[ C2U(".uno:Cut")] = bIsWritable && bControllerStateIsValid && m_apControllerState->bIsDeleteableObjectSelected;
    m_aCommandAvailability[ C2U(".uno:Copy")] = bControllerStateIsValid && m_apControllerState->bHasSelectedObject;
    m_aCommandAvailability[ C2U(".uno:Paste")] = bIsWritable && bHasSuitableClipboardContent;

    // toolbar commands
    m_aCommandAvailability[ C2U(".uno:ToggleGridHorizontal")] = bIsWritable;
    m_aCommandArguments[ C2U(".uno:ToggleGridHorizontal")] = uno::makeAny( m_apModelState->bHasMainYGrid );

    m_aCommandAvailability[ C2U(".uno:ToggleLegend")] = bIsWritable;
    m_aCommandArguments[ C2U(".uno:ToggleLegend")] = uno::makeAny( m_apModelState->bHasLegend );

    m_aCommandAvailability[ C2U(".uno:NewArrangement")] = bIsWritable;
    m_aCommandAvailability[ C2U(".uno:Update")] = bIsWritable;
    m_aCommandAvailability[ C2U(".uno:DefaultColors")] = bIsWritable;
    m_aCommandAvailability[ C2U(".uno:BarWidth")] = bIsWritable;
    m_aCommandAvailability[ C2U(".uno:NumberOfLines")] = bIsWritable;
    m_aCommandAvailability[ C2U(".uno:ArrangeRow")] = bIsWritable;

    // insert objects
    m_aCommandAvailability[ C2U(".uno:InsertTitle")] = bIsWritable;
    m_aCommandAvailability[ C2U(".uno:InsertLegend")] = bIsWritable;
    m_aCommandAvailability[ C2U(".uno:InsertDescription")] = bIsWritable;
    m_aCommandAvailability[ C2U(".uno:InsertAxis")] = bIsWritable && m_apModelState->bSupportsAxes;
    m_aCommandAvailability[ C2U(".uno:InsertGrids")] = bIsWritable && m_apModelState->bSupportsAxes;
//     m_aCommandAvailability[ C2U(".uno:InsertStatistics")] = bIsWritable && m_apModelState->bSupportsStatistics;
    m_aCommandAvailability[ C2U(".uno:InsertTrendlines")] = bIsWritable && m_apModelState->bSupportsStatistics;
    m_aCommandAvailability[ C2U(".uno:InsertMeanValues")] = bIsWritable && m_apModelState->bSupportsStatistics;
    m_aCommandAvailability[ C2U(".uno:InsertYErrorbars")] = bIsWritable && m_apModelState->bSupportsStatistics;
    m_aCommandAvailability[ C2U(".uno:InsertSymbol")] = bIsWritable && m_apControllerState->bIsTextObject;

    // format objects
    m_aCommandAvailability[ C2U(".uno:DiagramObjects")] = bIsWritable && bControllerStateIsValid && m_apControllerState->bIsFormateableObjectSelected;
    m_aCommandAvailability[ C2U(".uno:DiagramType")] = bIsWritable;
    m_aCommandAvailability[ C2U(".uno:Legend")] = bIsWritable && m_apModelState->bHasLegend;
    m_aCommandAvailability[ C2U(".uno:DiagramWall")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasWall;
    m_aCommandAvailability[ C2U(".uno:DiagramArea")] = bIsWritable;
    m_aCommandAvailability[ C2U(".uno:TransformDialog")] = bIsWritable && bControllerStateIsValid && m_apControllerState->bHasSelectedObject && m_apControllerState->bIsDraggableObject;

    // 3d commands
    m_aCommandAvailability[ C2U(".uno:View3D")] = bIsWritable && bModelStateIsValid && m_apModelState->bIsThreeD;
    m_aCommandAvailability[ C2U(".uno:DiagramFloor")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasFloor;

    // depending on own data
    m_aCommandAvailability[ C2U(".uno:DataRanges")] = bIsWritable && bModelStateIsValid && (! m_apModelState->bHasOwnData);
    m_aCommandAvailability[ C2U(".uno:DiagramData")] = bIsWritable && bModelStateIsValid &&  m_apModelState->bHasOwnData;

    // titles
    m_aCommandAvailability[ C2U(".uno:MainTitle")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasMainTitle;
    m_aCommandAvailability[ C2U(".uno:SubTitle")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasSubTitle;
    m_aCommandAvailability[ C2U(".uno:XTitle")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasXAxisTitle;
    m_aCommandAvailability[ C2U(".uno:YTitle")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasYAxisTitle;
    m_aCommandAvailability[ C2U(".uno:ZTitle")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasZAxisTitle;
    m_aCommandAvailability[ C2U(".uno:SecondaryXTitle")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasSecondaryXAxisTitle;
    m_aCommandAvailability[ C2U(".uno:SecondaryYTitle")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasSecondaryYAxisTitle;
    m_aCommandAvailability[ C2U(".uno:AllTitles")] = bIsWritable && bModelStateIsValid && m_apModelState->HasAnyTitle();

    // text
    m_aCommandAvailability[ C2U(".uno:ScaleText")] = bIsWritable && bModelStateIsValid ;

    // axes
    m_aCommandAvailability[ C2U(".uno:DiagramAxisX")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasXAxis;
    m_aCommandAvailability[ C2U(".uno:DiagramAxisY")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasYAxis;
    m_aCommandAvailability[ C2U(".uno:DiagramAxisZ")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasZAxis;
    m_aCommandAvailability[ C2U(".uno:DiagramAxisA")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasAAxis;
    m_aCommandAvailability[ C2U(".uno:DiagramAxisB")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasBAxis;
    m_aCommandAvailability[ C2U(".uno:DiagramAxisAll")] = bIsWritable && bModelStateIsValid && m_apModelState->HasAnyAxis();

    // grids
    // note: x and y are swapped in the commands!
    m_aCommandAvailability[ C2U(".uno:DiagramGridYMain")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasMainXGrid;
    m_aCommandAvailability[ C2U(".uno:DiagramGridXMain")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasMainYGrid;
    m_aCommandAvailability[ C2U(".uno:DiagramGridZMain")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasMainZGrid;
    m_aCommandAvailability[ C2U(".uno:DiagramGridYHelp")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasHelpXGrid;
    m_aCommandAvailability[ C2U(".uno:DiagramGridXHelp")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasHelpYGrid;
    m_aCommandAvailability[ C2U(".uno:DiagramGridZHelp")] = bIsWritable && bModelStateIsValid && m_apModelState->bHasHelpZGrid;
    m_aCommandAvailability[ C2U(".uno:DiagramGridAll")] = bIsWritable && bModelStateIsValid && m_apModelState->HasAnyGrid();

    // series arrangement
    m_aCommandAvailability[ C2U(".uno:Forward")] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayMoveSeriesForward;
    m_aCommandAvailability[ C2U(".uno:Backward")] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayMoveSeriesBackward;

    m_aCommandAvailability[ C2U(".uno:InsertMeanValue")] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayAddMeanValue;
    m_aCommandAvailability[ C2U(".uno:InsertTrendline")] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayAddTrendline;
    m_aCommandAvailability[ C2U(".uno:InsertTrendlineEquation")] = bIsWritable && bControllerStateIsValid && m_apControllerState->bMayAddTrendlineEquation;
}

bool ControllerCommandDispatch::commandAvailable( const OUString & rCommand )
{
    ::std::map< OUString, bool >::const_iterator aIt( m_aCommandAvailability.find( rCommand ));
    if( aIt != m_aCommandAvailability.end())
        return aIt->second;
    OSL_ENSURE( false, "commandAvailable: command not in availability map" );
    return false;
}

void ControllerCommandDispatch::fireStatusEvent(
    const OUString & rURL,
    const Reference< frame::XStatusListener > & xSingleListener /* = 0 */ )
{
    if( rURL.getLength() == 0 )
        for( ::std::map< OUString, bool >::const_iterator aIt( m_aCommandAvailability.begin());
             aIt != m_aCommandAvailability.end(); ++aIt )
            fireStatusEventForURLImpl( aIt->first, xSingleListener );
    else
        fireStatusEventForURLImpl( rURL, xSingleListener );

    // statusbar. Should be handled by base implementation
    // @todo: remove if Issue 68864 is fixed
    if( rURL.getLength() == 0 ||
        rURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(".uno:StatusBarVisible")))
    {
        bool bIsStatusBarVisible( lcl_isStatusBarVisible( m_xController ));
        fireStatusEventForURL( C2U(".uno:StatusBarVisible"), uno::makeAny( bIsStatusBarVisible ), true, xSingleListener );
    }
}

// ____ XDispatch ____
void SAL_CALL ControllerCommandDispatch::dispatch(
    const util::URL& URL,
    const Sequence< beans::PropertyValue >& Arguments )
    throw (uno::RuntimeException)
{
    if( commandAvailable( URL.Complete ))
        m_xDispatch->dispatch( URL, Arguments );
}

// ____ WeakComponentImplHelperBase ____
/// is called when this is disposed
void SAL_CALL ControllerCommandDispatch::disposing()
{
    m_xController.clear();
    m_xDispatch.clear();
    m_xSelectionSupplier.clear();
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL ControllerCommandDispatch::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    m_xController.clear();
    m_xDispatch.clear();
    m_xSelectionSupplier.clear();
}

// ____ XModifyListener ____
void SAL_CALL ControllerCommandDispatch::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    bool bUpdateCommandAvailability = false;

    // Update the "ModelState" Struct.
    if( m_apModelState.get() && m_xController.is())
    {
        m_apModelState->update( m_xController->getModel());
        bUpdateCommandAvailability = true;
    }

    // Update the "ControllerState" Struct.
    if( m_apControllerState.get() && m_xController.is())
    {
        m_apControllerState->update( m_xController, m_xController->getModel());
        bUpdateCommandAvailability = true;
    }

    if( bUpdateCommandAvailability )
        updateCommandAvailability();

    CommandDispatch::modified( aEvent );
}


// ____ XSelectionChangeListener ____
void SAL_CALL ControllerCommandDispatch::selectionChanged( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    // Update the "ControllerState" Struct.
    if( m_apControllerState.get() && m_xController.is())
    {
        m_apControllerState->update( m_xController, m_xController->getModel());
        updateCommandAvailability();
    }

    CommandDispatch::modified( aEvent );
}

} //  namespace chart
