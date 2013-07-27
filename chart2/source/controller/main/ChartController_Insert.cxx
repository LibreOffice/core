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

#include "dlg_InsertAxis_Grid.hxx"
#include "dlg_InsertDataLabel.hxx"
#include "dlg_InsertLegend.hxx"
#include "dlg_InsertErrorBars.hxx"
#include "dlg_InsertTitle.hxx"
#include "dlg_ObjectProperties.hxx"

#include "ChartWindow.hxx"
#include "ChartModelHelper.hxx"
#include "AxisHelper.hxx"
#include "TitleHelper.hxx"
#include "DiagramHelper.hxx"
#include "macros.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "NumberFormatterWrapper.hxx"
#include "ViewElementListProvider.hxx"
#include "MultipleChartConverters.hxx"
#include "ControllerLockGuard.hxx"
#include "UndoGuard.hxx"
#include "ResId.hxx"
#include "Strings.hrc"
#include "ReferenceSizeProvider.hxx"
#include "ObjectIdentifier.hxx"
#include "RegressionCurveHelper.hxx"
#include "RegressionCurveItemConverter.hxx"
#include "StatisticsHelper.hxx"
#include "ErrorBarItemConverter.hxx"
#include "MultipleItemConverter.hxx"
#include "DataSeriesHelper.hxx"
#include "ObjectNameProvider.hxx"
#include "LegendHelper.hxx"

#include <com/sun/star/chart2/XRegressionCurve.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <svx/ActionDescriptionProvider.hxx>

//--------------------------------------

// header for define RET_OK
#include <vcl/msgbox.hxx>
// header for class OUStringBuffer
#include <rtl/ustrbuf.hxx>
// header for class Application
#include <vcl/svapp.hxx>
// header for class ::vos::OGuard
#include <osl/mutex.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

//.............................................................................

namespace
{
struct lcl_InsertMeanValueLine
{
public:
    lcl_InsertMeanValueLine( const uno::Reference< uno::XComponentContext > & xContext ) :
            m_xContext( xContext )
    {}

    void operator()( const uno::Reference< chart2::XDataSeries > & xSeries )
    {
        uno::Reference< chart2::XRegressionCurveContainer > xRegCurveCnt(
            xSeries, uno::UNO_QUERY );
        if( xRegCurveCnt.is())
        {
            ::chart::RegressionCurveHelper::addMeanValueLine(
                xRegCurveCnt, m_xContext, uno::Reference< beans::XPropertySet >( xSeries, uno::UNO_QUERY ));
        }
    }

private:
    uno::Reference< uno::XComponentContext > m_xContext;
};

} // anonymous namespace

//.............................................................................
namespace chart
{
//.............................................................................

void ChartController::executeDispatch_InsertAxes()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, SCH_RESSTR( STR_OBJECT_AXES )),
        m_xUndoManager );

    try
    {
        InsertAxisOrGridDialogData aDialogInput;
        uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram(getModel());
        AxisHelper::getAxisOrGridExcistence( aDialogInput.aExistenceList, xDiagram, sal_True );
        AxisHelper::getAxisOrGridPossibilities( aDialogInput.aPossibilityList, xDiagram, sal_True );

        SolarMutexGuard aGuard;
        SchAxisDlg aDlg( m_pChartWindow, aDialogInput );
        if( aDlg.Execute() == RET_OK )
        {
            // lock controllers till end of block
            ControllerLockGuard aCLGuard( getModel() );

            InsertAxisOrGridDialogData aDialogOutput;
            aDlg.getResult( aDialogOutput );
            ::std::auto_ptr< ReferenceSizeProvider > mpRefSizeProvider(
                impl_createReferenceSizeProvider());
            bool bChanged = AxisHelper::changeVisibilityOfAxes( xDiagram
                , aDialogInput.aExistenceList, aDialogOutput.aExistenceList, m_xCC
                , mpRefSizeProvider.get() );
            if( bChanged )
                aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void ChartController::executeDispatch_InsertGrid()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, SCH_RESSTR( STR_OBJECT_GRIDS )),
        m_xUndoManager );

    try
    {
        InsertAxisOrGridDialogData aDialogInput;
        uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram(getModel());
        AxisHelper::getAxisOrGridExcistence( aDialogInput.aExistenceList, xDiagram, sal_False );
        AxisHelper::getAxisOrGridPossibilities( aDialogInput.aPossibilityList, xDiagram, sal_False );

        SolarMutexGuard aGuard;
        SchGridDlg aDlg( m_pChartWindow, aDialogInput );//aItemSet, b3D, bNet, bSecondaryX, bSecondaryY );
        if( aDlg.Execute() == RET_OK )
        {
            // lock controllers till end of block
            ControllerLockGuard aCLGuard( getModel() );
            InsertAxisOrGridDialogData aDialogOutput;
            aDlg.getResult( aDialogOutput );
            bool bChanged = AxisHelper::changeVisibilityOfGrids( xDiagram
                , aDialogInput.aExistenceList, aDialogOutput.aExistenceList, m_xCC );
            if( bChanged )
                aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void ChartController::executeDispatch_InsertTitles()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, SCH_RESSTR( STR_OBJECT_TITLES )),
        m_xUndoManager );

    try
    {
        TitleDialogData aDialogInput;
        aDialogInput.readFromModel( getModel() );

        SolarMutexGuard aGuard;
        SchTitleDlg aDlg( m_pChartWindow, aDialogInput );
        if( aDlg.Execute() == RET_OK )
        {
            // lock controllers till end of block
            ControllerLockGuard aCLGuard( getModel() );
            TitleDialogData aDialogOutput( impl_createReferenceSizeProvider());
            aDlg.getResult( aDialogOutput );
            bool bChanged = aDialogOutput.writeDifferenceToModel( getModel(), m_xCC, &aDialogInput );
            if( bChanged )
                aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void ChartController::executeDispatch_DeleteLegend()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::DELETE, SCH_RESSTR( STR_OBJECT_LEGEND )),
        m_xUndoManager );

    LegendHelper::hideLegend( getModel() );
    aUndoGuard.commit();
}

void ChartController::executeDispatch_InsertLegend()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, SCH_RESSTR( STR_OBJECT_LEGEND )),
        m_xUndoManager );

    Reference< chart2::XLegend > xLegend = LegendHelper::showLegend( getModel(), m_xCC );
    aUndoGuard.commit();
}

void ChartController::executeDispatch_OpenLegendDialog()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, SCH_RESSTR( STR_OBJECT_LEGEND )),
        m_xUndoManager );

    try
    {
        //prepare and open dialog
        SolarMutexGuard aGuard;
        SchLegendDlg aDlg( m_pChartWindow, m_xCC );
        aDlg.init( getModel() );
        if( aDlg.Execute() == RET_OK )
        {
            // lock controllers till end of block
            ControllerLockGuard aCLGuard( getModel() );
            bool bChanged = aDlg.writeToModel( getModel() );
            if( bChanged )
                aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void ChartController::executeDispatch_InsertMenu_DataLabels()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, SCH_RESSTR( STR_OBJECT_DATALABELS )),
        m_xUndoManager );

    //if a series is selected insert labels for that series only:
    uno::Reference< chart2::XDataSeries > xSeries(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getModel()), uno::UNO_QUERY );
    if( xSeries.is() )
    {
        // add labels
        DataSeriesHelper::insertDataLabelsToSeriesAndAllPoints( xSeries );

        OUString aChildParticle( ObjectIdentifier::getStringForType( OBJECTTYPE_DATA_LABELS ) + "=" );
        OUString aObjectCID = ObjectIdentifier::createClassifiedIdentifierForParticles(
            ObjectIdentifier::getSeriesParticleFromCID(m_aSelection.getSelectedCID()), aChildParticle );

        bool bSuccess = ChartController::executeDlg_ObjectProperties_withoutUndoGuard( aObjectCID, true );
        if( bSuccess )
            aUndoGuard.commit();
        return;
    }

    try
    {
        wrapper::AllDataLabelItemConverter aItemConverter(
            getModel(),
            m_pDrawModelWrapper->GetItemPool(),
            m_pDrawModelWrapper->getSdrModel(),
            uno::Reference< lang::XMultiServiceFactory >( getModel(), uno::UNO_QUERY ));
        SfxItemSet aItemSet = aItemConverter.CreateEmptyItemSet();
        aItemConverter.FillItemSet( aItemSet );

        //prepare and open dialog
        SolarMutexGuard aGuard;

        //get number formatter
        uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( getModel(), uno::UNO_QUERY );
        NumberFormatterWrapper aNumberFormatterWrapper( xNumberFormatsSupplier );
        SvNumberFormatter* pNumberFormatter = aNumberFormatterWrapper.getSvNumberFormatter();

        DataLabelsDialog aDlg( m_pChartWindow, aItemSet, pNumberFormatter);

        if( aDlg.Execute() == RET_OK )
        {
            SfxItemSet aOutItemSet = aItemConverter.CreateEmptyItemSet();
            aDlg.FillItemSet( aOutItemSet );
            // lock controllers till end of block
            ControllerLockGuard aCLGuard( getModel() );
            bool bChanged = aItemConverter.ApplyItemSet( aOutItemSet );//model should be changed now
            if( bChanged )
                aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void ChartController::executeDispatch_InsertMeanValue()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, SCH_RESSTR( STR_OBJECT_AVERAGE_LINE )),
        m_xUndoManager );
    lcl_InsertMeanValueLine( m_xCC ).operator()(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getModel() ));
    aUndoGuard.commit();
}

void ChartController::executeDispatch_InsertMenu_MeanValues()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, SCH_RESSTR( STR_OBJECT_AVERAGE_LINE )),
        m_xUndoManager );

    uno::Reference< chart2::XDataSeries > xSeries(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getModel() ), uno::UNO_QUERY );
    if( xSeries.is() )
    {
        //if a series is selected insert mean value only for that series:
        lcl_InsertMeanValueLine( m_xCC ).operator()(xSeries);
    }
    else
    {
        ::std::vector< uno::Reference< chart2::XDataSeries > > aSeries(
            DiagramHelper::getDataSeriesFromDiagram( ChartModelHelper::findDiagram( getModel() )));
        ::std::for_each( aSeries.begin(), aSeries.end(), lcl_InsertMeanValueLine( m_xCC ));
    }
    aUndoGuard.commit();
}

void ChartController::executeDispatch_InsertMenu_Trendlines()
{
    OUString aCID = m_aSelection.getSelectedCID();

    uno::Reference< chart2::XDataSeries > xSeries(
        ObjectIdentifier::getDataSeriesForCID( aCID, getModel() ), uno::UNO_QUERY );

    if( !xSeries.is() )
        return;

    executeDispatch_InsertTrendline();
}

void ChartController::executeDispatch_InsertTrendline()
{
    uno::Reference< chart2::XRegressionCurveContainer > xRegressionCurveContainer(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getModel()), uno::UNO_QUERY );

    if( !xRegressionCurveContainer.is() )
        return;

    UndoLiveUpdateGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, SCH_RESSTR( STR_OBJECT_CURVE )),
        m_xUndoManager );

    uno::Reference< chart2::XRegressionCurve > xCurve =
        RegressionCurveHelper::addRegressionCurve(
            RegressionCurveHelper::REGRESSION_TYPE_LINEAR,
            xRegressionCurveContainer,
            m_xCC );

    uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );

    if( !xProperties.is())
        return;

    wrapper::RegressionCurveItemConverter aItemConverter(
        xProperties, xRegressionCurveContainer, m_pDrawModelWrapper->getSdrModel().GetItemPool(),
        m_pDrawModelWrapper->getSdrModel(),
        uno::Reference< lang::XMultiServiceFactory >( getModel(), uno::UNO_QUERY ));

    // open dialog
    SfxItemSet aItemSet = aItemConverter.CreateEmptyItemSet();
    aItemConverter.FillItemSet( aItemSet );
    ObjectPropertiesDialogParameter aDialogParameter = ObjectPropertiesDialogParameter(
        ObjectIdentifier::createDataCurveCID(
            ObjectIdentifier::getSeriesParticleFromCID( m_aSelection.getSelectedCID()),
            RegressionCurveHelper::getRegressionCurveIndex( xRegressionCurveContainer, xCurve ), false ));
    aDialogParameter.init( getModel() );
    ViewElementListProvider aViewElementListProvider( m_pDrawModelWrapper.get());
    SolarMutexGuard aGuard;
    SchAttribTabDlg aDialog( m_pChartWindow, &aItemSet, &aDialogParameter, &aViewElementListProvider,
                          uno::Reference< util::XNumberFormatsSupplier >( getModel(), uno::UNO_QUERY ));

    // note: when a user pressed "OK" but didn't change any settings in the
    // dialog, the SfxTabDialog returns "Cancel"
    if( aDialog.Execute() == RET_OK || aDialog.DialogWasClosedWithOK())
    {
        const SfxItemSet* pOutItemSet = aDialog.GetOutputItemSet();
        if( pOutItemSet )
        {
            ControllerLockGuard aCLGuard( getModel() );
            aItemConverter.ApplyItemSet( *pOutItemSet );
        }
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_InsertErrorBars( bool bYError )
{
    ObjectType objType = bYError ? OBJECTTYPE_DATA_ERRORS_Y : OBJECTTYPE_DATA_ERRORS_X;

    //if a series is selected insert error bars for that series only:
    uno::Reference< chart2::XDataSeries > xSeries(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getModel() ), uno::UNO_QUERY );

    if( xSeries.is())
    {
        UndoLiveUpdateGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::INSERT,
                SCH_RESSTR( bYError ? STR_OBJECT_ERROR_BARS_Y : STR_OBJECT_ERROR_BARS_X )),
            m_xUndoManager );

        // add error bars with standard deviation
        uno::Reference< beans::XPropertySet > xErrorBarProp(
            StatisticsHelper::addErrorBars( xSeries, m_xCC,
                                            ::com::sun::star::chart::ErrorBarStyle::STANDARD_DEVIATION,
                                            bYError));

        // get an appropriate item converter
        wrapper::ErrorBarItemConverter aItemConverter(
            getModel(), xErrorBarProp, m_pDrawModelWrapper->getSdrModel().GetItemPool(),
            m_pDrawModelWrapper->getSdrModel(),
            uno::Reference< lang::XMultiServiceFactory >( getModel(), uno::UNO_QUERY ));

        // open dialog
        SfxItemSet aItemSet = aItemConverter.CreateEmptyItemSet();
        aItemSet.Put(SfxBoolItem(SCHATTR_STAT_ERRORBAR_TYPE,bYError));
        aItemConverter.FillItemSet( aItemSet );
        ObjectPropertiesDialogParameter aDialogParameter = ObjectPropertiesDialogParameter(
            ObjectIdentifier::createClassifiedIdentifierWithParent(
                objType, OUString(), m_aSelection.getSelectedCID()));
        aDialogParameter.init( getModel() );
        ViewElementListProvider aViewElementListProvider( m_pDrawModelWrapper.get());
        SolarMutexGuard aGuard;
        SchAttribTabDlg aDlg( m_pChartWindow, &aItemSet, &aDialogParameter, &aViewElementListProvider,
                              uno::Reference< util::XNumberFormatsSupplier >( getModel(), uno::UNO_QUERY ));
        aDlg.SetAxisMinorStepWidthForErrorBarDecimals(
            InsertErrorBarsDialog::getAxisMinorStepWidthForErrorBarDecimals( getModel(),
                                                                             m_xChartView, m_aSelection.getSelectedCID()));

        // note: when a user pressed "OK" but didn't change any settings in the
        // dialog, the SfxTabDialog returns "Cancel"
        if( aDlg.Execute() == RET_OK || aDlg.DialogWasClosedWithOK())
        {
            const SfxItemSet* pOutItemSet = aDlg.GetOutputItemSet();
            if( pOutItemSet )
            {
                ControllerLockGuard aCLGuard( getModel() );
                aItemConverter.ApplyItemSet( *pOutItemSet );
            }
            aUndoGuard.commit();
        }
    }
    else
    {
        //if no series is selected insert error bars for all series
        UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::INSERT,
                ObjectNameProvider::getName_ObjectForAllSeries( objType ) ),
            m_xUndoManager );

        try
        {
            wrapper::AllSeriesStatisticsConverter aItemConverter(
                getModel(), m_pDrawModelWrapper->GetItemPool() );
            SfxItemSet aItemSet = aItemConverter.CreateEmptyItemSet();
            aItemConverter.FillItemSet( aItemSet );

            //prepare and open dialog
            SolarMutexGuard aGuard;
            InsertErrorBarsDialog aDlg(
                m_pChartWindow, aItemSet,
                uno::Reference< chart2::XChartDocument >( getModel(), uno::UNO_QUERY ),
                bYError ? ErrorBarResources::ERROR_BAR_Y : ErrorBarResources::ERROR_BAR_X);

            aDlg.SetAxisMinorStepWidthForErrorBarDecimals(
                InsertErrorBarsDialog::getAxisMinorStepWidthForErrorBarDecimals( getModel(), m_xChartView, OUString() ) );

            if( aDlg.Execute() == RET_OK )
            {
                SfxItemSet aOutItemSet = aItemConverter.CreateEmptyItemSet();
                aDlg.FillItemSet( aOutItemSet );

                // lock controllers till end of block
                ControllerLockGuard aCLGuard( getModel() );
                bool bChanged = aItemConverter.ApplyItemSet( aOutItemSet );//model should be changed now
                if( bChanged )
                    aUndoGuard.commit();
            }
        }
        catch(const uno::RuntimeException& e)
        {
            ASSERT_EXCEPTION( e );
        }
    }
}

void ChartController::executeDispatch_InsertTrendlineEquation( bool bInsertR2 )
{
    uno::Reference< chart2::XRegressionCurve > xRegCurve(
        ObjectIdentifier::getObjectPropertySet( m_aSelection.getSelectedCID(), getModel() ), uno::UNO_QUERY );
    if( !xRegCurve.is() )
    {
        uno::Reference< chart2::XRegressionCurveContainer > xRegCurveCnt(
            ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getModel() ), uno::UNO_QUERY );
        xRegCurve.set( RegressionCurveHelper::getFirstCurveNotMeanValueLine( xRegCurveCnt ) );
    }
    if( xRegCurve.is())
    {
        uno::Reference< beans::XPropertySet > xEqProp( xRegCurve->getEquationProperties());
        if( xEqProp.is())
        {
            // using assignment for broken gcc 3.3
            UndoGuard aUndoGuard = UndoGuard(
                ActionDescriptionProvider::createDescription(
                    ActionDescriptionProvider::INSERT, SCH_RESSTR( STR_OBJECT_CURVE_EQUATION )),
                m_xUndoManager );
            xEqProp->setPropertyValue( "ShowEquation", uno::makeAny( true ));
            xEqProp->setPropertyValue( "ShowCorrelationCoefficient", uno::makeAny( bInsertR2 ));
            aUndoGuard.commit();
        }
    }
}

void ChartController::executeDispatch_InsertR2Value()
{
    uno::Reference< beans::XPropertySet > xEqProp(
        ObjectIdentifier::getObjectPropertySet( m_aSelection.getSelectedCID(), getModel() ), uno::UNO_QUERY );
    if( xEqProp.is())
    {
        UndoGuard aUndoGuard = UndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::INSERT, SCH_RESSTR( STR_OBJECT_CURVE_EQUATION )),
            m_xUndoManager );
        xEqProp->setPropertyValue( "ShowCorrelationCoefficient", uno::makeAny( true ));
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_DeleteR2Value()
{
    uno::Reference< beans::XPropertySet > xEqProp(
        ObjectIdentifier::getObjectPropertySet( m_aSelection.getSelectedCID(), getModel() ), uno::UNO_QUERY );
    if( xEqProp.is())
    {
        UndoGuard aUndoGuard = UndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::INSERT, SCH_RESSTR( STR_OBJECT_CURVE_EQUATION )),
            m_xUndoManager );
        xEqProp->setPropertyValue( "ShowCorrelationCoefficient", uno::makeAny( false ));
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_DeleteMeanValue()
{
    uno::Reference< chart2::XRegressionCurveContainer > xRegCurveCnt(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getModel() ), uno::UNO_QUERY );
    if( xRegCurveCnt.is())
    {
        UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::DELETE, SCH_RESSTR( STR_OBJECT_AVERAGE_LINE )),
            m_xUndoManager );
        RegressionCurveHelper::removeMeanValueLine( xRegCurveCnt );
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_DeleteTrendline()
{
    uno::Reference< chart2::XRegressionCurveContainer > xRegCurveCnt(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getModel() ), uno::UNO_QUERY );
    if( xRegCurveCnt.is())
    {
        UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::DELETE, SCH_RESSTR( STR_OBJECT_CURVE )),
            m_xUndoManager );
        RegressionCurveHelper::removeAllExceptMeanValueLine( xRegCurveCnt );
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_DeleteTrendlineEquation()
{
    uno::Reference< chart2::XRegressionCurveContainer > xRegCurveCnt(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getModel() ), uno::UNO_QUERY );
    if( xRegCurveCnt.is())
    {
        UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::DELETE, SCH_RESSTR( STR_OBJECT_CURVE_EQUATION )),
            m_xUndoManager );
        RegressionCurveHelper::removeEquations( xRegCurveCnt );
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_DeleteErrorBars( bool bYError )
{
    uno::Reference< chart2::XDataSeries > xDataSeries(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getModel() ));
    if( xDataSeries.is())
    {
        UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::DELETE, SCH_RESSTR( STR_OBJECT_CURVE )),
            m_xUndoManager );
        StatisticsHelper::removeErrorBars( xDataSeries, bYError );
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_InsertDataLabels()
{
    uno::Reference< chart2::XDataSeries > xSeries(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getModel() ), uno::UNO_QUERY );
    if( xSeries.is() )
    {
        UndoGuard aUndoGuard = UndoGuard( ActionDescriptionProvider::createDescription( ActionDescriptionProvider::INSERT,
            SCH_RESSTR( STR_OBJECT_DATALABELS )),
            m_xUndoManager );
        DataSeriesHelper::insertDataLabelsToSeriesAndAllPoints( xSeries );
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_InsertDataLabel()
{
    UndoGuard aUndoGuard = UndoGuard( ActionDescriptionProvider::createDescription( ActionDescriptionProvider::INSERT,
        SCH_RESSTR( STR_OBJECT_LABEL )),
        m_xUndoManager );
    DataSeriesHelper::insertDataLabelToPoint( ObjectIdentifier::getObjectPropertySet( m_aSelection.getSelectedCID(), getModel() ) );
    aUndoGuard.commit();
}

void ChartController::executeDispatch_DeleteDataLabels()
{
    uno::Reference< chart2::XDataSeries > xSeries(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getModel() ), uno::UNO_QUERY );
    if( xSeries.is() )
    {
        UndoGuard aUndoGuard( ActionDescriptionProvider::createDescription( ActionDescriptionProvider::DELETE,
            SCH_RESSTR( STR_OBJECT_DATALABELS )),
            m_xUndoManager );
        DataSeriesHelper::deleteDataLabelsFromSeriesAndAllPoints( xSeries );
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_DeleteDataLabel()
{
    UndoGuard aUndoGuard( ActionDescriptionProvider::createDescription( ActionDescriptionProvider::DELETE,
        SCH_RESSTR( STR_OBJECT_LABEL )),
        m_xUndoManager );
    DataSeriesHelper::deleteDataLabelsFromPoint( ObjectIdentifier::getObjectPropertySet( m_aSelection.getSelectedCID(), getModel() ) );
    aUndoGuard.commit();
}

void ChartController::executeDispatch_ResetAllDataPoints()
{
    UndoGuard aUndoGuard( ActionDescriptionProvider::createDescription( ActionDescriptionProvider::FORMAT,
        SCH_RESSTR( STR_OBJECT_DATAPOINTS )),
        m_xUndoManager );
    uno::Reference< chart2::XDataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getModel() ), uno::UNO_QUERY );
    if( xSeries.is() )
        xSeries->resetAllDataPoints();
    aUndoGuard.commit();
}
void ChartController::executeDispatch_ResetDataPoint()
{
    UndoGuard aUndoGuard( ActionDescriptionProvider::createDescription( ActionDescriptionProvider::FORMAT,
        SCH_RESSTR( STR_OBJECT_DATAPOINT )),
        m_xUndoManager );
    uno::Reference< chart2::XDataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getModel() ), uno::UNO_QUERY );
    if( xSeries.is() )
    {
        sal_Int32 nPointIndex = ObjectIdentifier::getIndexFromParticleOrCID( m_aSelection.getSelectedCID() );
        xSeries->resetDataPoint( nPointIndex );
    }
    aUndoGuard.commit();
}

void ChartController::executeDispatch_InsertAxisTitle()
{
    try
    {
        uno::Reference< XTitle > xTitle;
        {
            UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::INSERT, SCH_RESSTR( STR_OBJECT_TITLE )),
            m_xUndoManager );

            Reference< XAxis > xAxis = ObjectIdentifier::getAxisForCID( m_aSelection.getSelectedCID(), getModel() );
            sal_Int32 nDimensionIndex = -1;
            sal_Int32 nCooSysIndex = -1;
            sal_Int32 nAxisIndex = -1;
            AxisHelper::getIndicesForAxis( xAxis, ChartModelHelper::findDiagram(getModel()), nCooSysIndex, nDimensionIndex, nAxisIndex );

            TitleHelper::eTitleType eTitleType = TitleHelper::X_AXIS_TITLE;
            if( nDimensionIndex==0 )
                eTitleType = nAxisIndex==0 ? TitleHelper::X_AXIS_TITLE : TitleHelper::SECONDARY_X_AXIS_TITLE;
            else if( nDimensionIndex==1 )
                eTitleType = nAxisIndex==0 ? TitleHelper::Y_AXIS_TITLE : TitleHelper::SECONDARY_Y_AXIS_TITLE;
            else
                eTitleType = TitleHelper::Z_AXIS_TITLE;

            ::std::auto_ptr< ReferenceSizeProvider > apRefSizeProvider( impl_createReferenceSizeProvider());
            xTitle = TitleHelper::createTitle( eTitleType, ObjectNameProvider::getTitleNameByType(eTitleType), getModel(), m_xCC, apRefSizeProvider.get() );
            aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void ChartController::executeDispatch_InsertAxis()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, SCH_RESSTR( STR_OBJECT_AXIS )),
        m_xUndoManager );

    try
    {
        Reference< XAxis > xAxis = ObjectIdentifier::getAxisForCID( m_aSelection.getSelectedCID(), getModel() );
        if( xAxis.is() )
        {
            AxisHelper::makeAxisVisible( xAxis );
            aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void ChartController::executeDispatch_DeleteAxis()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::DELETE, SCH_RESSTR( STR_OBJECT_AXIS )),
        m_xUndoManager );

    try
    {
        Reference< XAxis > xAxis = ObjectIdentifier::getAxisForCID( m_aSelection.getSelectedCID(), getModel() );
        if( xAxis.is() )
        {
            AxisHelper::makeAxisInvisible( xAxis );
            aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void ChartController::executeDispatch_InsertMajorGrid()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, SCH_RESSTR( STR_OBJECT_GRID )),
        m_xUndoManager );

    try
    {
        Reference< XAxis > xAxis = ObjectIdentifier::getAxisForCID( m_aSelection.getSelectedCID(), getModel() );
        if( xAxis.is() )
        {
            AxisHelper::makeGridVisible( xAxis->getGridProperties() );
            aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void ChartController::executeDispatch_DeleteMajorGrid()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::DELETE, SCH_RESSTR( STR_OBJECT_GRID )),
        m_xUndoManager );

    try
    {
        Reference< XAxis > xAxis = ObjectIdentifier::getAxisForCID( m_aSelection.getSelectedCID(), getModel() );
        if( xAxis.is() )
        {
            AxisHelper::makeGridInvisible( xAxis->getGridProperties() );
            aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void ChartController::executeDispatch_InsertMinorGrid()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, SCH_RESSTR( STR_OBJECT_GRID )),
        m_xUndoManager );

    try
    {
        Reference< XAxis > xAxis = ObjectIdentifier::getAxisForCID( m_aSelection.getSelectedCID(), getModel() );
        if( xAxis.is() )
        {
            Sequence< Reference< beans::XPropertySet > > aSubGrids( xAxis->getSubGridProperties() );
            for( sal_Int32 nN=0; nN<aSubGrids.getLength(); nN++)
                AxisHelper::makeGridVisible( aSubGrids[nN] );
            aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void ChartController::executeDispatch_DeleteMinorGrid()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::DELETE, SCH_RESSTR(STR_OBJECT_GRID)),
        m_xUndoManager );

    try
    {
        Reference< XAxis > xAxis = ObjectIdentifier::getAxisForCID( m_aSelection.getSelectedCID(), getModel() );
        if( xAxis.is() )
        {
            Sequence< Reference< beans::XPropertySet > > aSubGrids( xAxis->getSubGridProperties() );
            for( sal_Int32 nN=0; nN<aSubGrids.getLength(); nN++)
                AxisHelper::makeGridInvisible( aSubGrids[nN] );
            aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
