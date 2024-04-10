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

#include <memory>
#include <ChartController.hxx>

#include <dlg_InsertAxis_Grid.hxx>
#include <dlg_InsertDataLabel.hxx>
#include <dlg_InsertLegend.hxx>
#include <dlg_InsertErrorBars.hxx>
#include <dlg_InsertTitle.hxx>
#include <dlg_InsertDataTable.hxx>
#include <dlg_ObjectProperties.hxx>

#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <TitleHelper.hxx>
#include <DataSeries.hxx>
#include <Diagram.hxx>
#include <GridProperties.hxx>
#include <chartview/DrawModelWrapper.hxx>
#include <chartview/ChartSfxItemIds.hxx>
#include <NumberFormatterWrapper.hxx>
#include <ViewElementListProvider.hxx>
#include <MultipleChartConverters.hxx>
#include <ControllerLockGuard.hxx>
#include "UndoGuard.hxx"
#include <ResId.hxx>
#include <strings.hrc>
#include <ReferenceSizeProvider.hxx>
#include <ObjectIdentifier.hxx>
#include <RegressionCurveHelper.hxx>
#include <RegressionCurveItemConverter.hxx>
#include <StatisticsHelper.hxx>
#include <ErrorBarItemConverter.hxx>
#include <DataSeriesHelper.hxx>
#include <ObjectNameProvider.hxx>
#include <Legend.hxx>
#include <LegendHelper.hxx>
#include <DataTable.hxx>
#include <RegressionCurveModel.hxx>

#include <com/sun/star/chart2/XRegressionCurve.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <svx/ActionDescriptionProvider.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

namespace
{

void lcl_InsertMeanValueLine( const rtl::Reference< ::chart::DataSeries > & xSeries )
{
    if( xSeries.is())
    {
        ::chart::RegressionCurveHelper::addMeanValueLine(
            xSeries, xSeries);
    }
}

} // anonymous namespace

namespace chart
{

void ChartController::executeDispatch_InsertAxes()
{
    auto xUndoGuard = std::make_shared<UndoGuard>(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Insert, SchResId( STR_OBJECT_AXES )),
        m_xUndoManager );

    try
    {
        auto aDialogInput = std::make_shared<InsertAxisOrGridDialogData>();
        rtl::Reference< Diagram > xDiagram = getFirstDiagram();
        AxisHelper::getAxisOrGridExistence( aDialogInput->aExistenceList, xDiagram );
        AxisHelper::getAxisOrGridPossibilities( aDialogInput->aPossibilityList, xDiagram );

        SolarMutexGuard aGuard;
        auto aDlg = std::make_shared<SchAxisDlg>(GetChartFrame(), *aDialogInput);
        weld::DialogController::runAsync(aDlg, [this, aDlg, aDialogInput, xUndoGuard=std::move(xUndoGuard)](int nResult) {
            if ( nResult == RET_OK )
            {
                // lock controllers till end of block
                ControllerLockGuardUNO aCLGuard( getChartModel() );

                InsertAxisOrGridDialogData aDialogOutput;
                aDlg->getResult(aDialogOutput);
                ReferenceSizeProvider aRefSizeProvider(impl_createReferenceSizeProvider());
                bool bChanged = AxisHelper::changeVisibilityOfAxes( getFirstDiagram()
                    , aDialogInput->aExistenceList, aDialogOutput.aExistenceList, m_xCC
                    , &aRefSizeProvider );
                if( bChanged )
                    xUndoGuard->commit();
            }
        });
    }
    catch(const uno::RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

void ChartController::executeDispatch_InsertGrid()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Insert, SchResId( STR_OBJECT_GRIDS )),
        m_xUndoManager );

    try
    {
        InsertAxisOrGridDialogData aDialogInput;
        rtl::Reference< Diagram > xDiagram = getFirstDiagram();
        AxisHelper::getAxisOrGridExistence( aDialogInput.aExistenceList, xDiagram, false );
        AxisHelper::getAxisOrGridPossibilities( aDialogInput.aPossibilityList, xDiagram, false );

        SolarMutexGuard aGuard;
        SchGridDlg aDlg(GetChartFrame(), aDialogInput);//aItemSet, b3D, bNet, bSecondaryX, bSecondaryY );
        if (aDlg.run() == RET_OK)
        {
            // lock controllers till end of block
            ControllerLockGuardUNO aCLGuard( getChartModel() );
            InsertAxisOrGridDialogData aDialogOutput;
            aDlg.getResult( aDialogOutput );
            bool bChanged = AxisHelper::changeVisibilityOfGrids( xDiagram
                , aDialogInput.aExistenceList, aDialogOutput.aExistenceList );
            if( bChanged )
                aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

void ChartController::executeDispatch_OpenInsertDataTableDialog()
{
    SolarMutexGuard aGuard;

    auto aUndoDescription = ActionDescriptionProvider::createDescription(ActionDescriptionProvider::ActionType::Insert, SchResId(STR_DATA_TABLE));
    UndoGuard aUndoGuard(aUndoDescription, m_xUndoManager);

    rtl::Reference<Diagram> xDiagram = getFirstDiagram();

    InsertDataTableDialog aDialog(GetChartFrame());
    {
        // init values
        DataTableDialogData aData;
        auto xDataTable = xDiagram->getDataTable();
        aData.mbShow = xDataTable.is();
        if (xDataTable.is())
        {
            uno::Reference<beans::XPropertySet> xProperties(xDataTable, uno::UNO_QUERY);

            uno::Any aAny = xProperties->getPropertyValue(u"HBorder"_ustr);
            if (aAny.has<bool>())
                aData.mbHorizontalBorders = aAny.get<bool>();

            aAny = xProperties->getPropertyValue(u"VBorder"_ustr);
            if (aAny.has<bool>())
                aData.mbVerticalBorders = aAny.get<bool>();

            aAny = xProperties->getPropertyValue(u"Outline"_ustr);
            if (aAny.has<bool>())
                aData.mbOutline = aAny.get<bool>();

            aAny = xProperties->getPropertyValue(u"Keys"_ustr);
            if (aAny.has<bool>())
                aData.mbKeys = aAny.get<bool>();
        }
        aDialog.init(aData);
    }

    // show the dialog
    if (aDialog.run() == RET_OK)
    {
        bool bChanged = false;

        auto& rDialogData = aDialog.getDataTableDialogData();
        auto xDataTable = xDiagram->getDataTable();
        if (!rDialogData.mbShow && xDataTable.is())
        {
            xDiagram->setDataTable(uno::Reference<chart2::XDataTable>());
            bChanged = true;
        }
        else if (rDialogData.mbShow && !xDataTable.is())
        {
            uno::Reference<chart2::XDataTable> xNewDataTable(new DataTable);
            if (xNewDataTable.is())
            {
                xDiagram->setDataTable(xNewDataTable);
                bChanged = true;
            }
        }

        // Set the properties
        xDataTable = xDiagram->getDataTable();
        if (rDialogData.mbShow && xDataTable.is())
        {
            uno::Reference<beans::XPropertySet> xProperties(xDataTable, uno::UNO_QUERY);
            xProperties->setPropertyValue(u"HBorder"_ustr , uno::Any(rDialogData.mbHorizontalBorders));
            xProperties->setPropertyValue(u"VBorder"_ustr , uno::Any(rDialogData.mbVerticalBorders));
            xProperties->setPropertyValue(u"Outline"_ustr , uno::Any(rDialogData.mbOutline));
            xProperties->setPropertyValue(u"Keys"_ustr , uno::Any(rDialogData.mbKeys));
            bChanged = true;
        }

        if (bChanged)
            aUndoGuard.commit();
    }
}

/** Create and insert a data table to the chart */
void ChartController::executeDispatch_InsertDataTable()
{
    auto aUndoDescription = ActionDescriptionProvider::createDescription(ActionDescriptionProvider::ActionType::Insert, SchResId(STR_DATA_TABLE));
    UndoGuard aUndoGuard(aUndoDescription, m_xUndoManager);


    rtl::Reference<Diagram> xDiagram = getFirstDiagram();
    auto xDataTable = xDiagram->getDataTable();
    if (!xDataTable.is())
    {
        uno::Reference<chart2::XDataTable> xNewDataTable(new DataTable);
        if (xNewDataTable.is())
        {
            xDiagram->setDataTable(xNewDataTable);
            aUndoGuard.commit();
        }
    }
}

/** Delete a data table from the chart */
void ChartController::executeDispatch_DeleteDataTable()
{
    auto aUndoDescription = ActionDescriptionProvider::createDescription(ActionDescriptionProvider::ActionType::Delete, SchResId(STR_DATA_TABLE));
    UndoGuard aUndoGuard(aUndoDescription, m_xUndoManager);

    rtl::Reference<Diagram> xDiagram = getFirstDiagram();
    auto xDataTable = xDiagram->getDataTable();
    if (xDataTable.is())
    {
        // insert a empty data table reference
        xDiagram->setDataTable(uno::Reference<chart2::XDataTable>());
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_InsertTitles()
{
    auto xUndoGuard = std::make_shared<UndoGuard>(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Insert, SchResId( STR_OBJECT_TITLES )),
        m_xUndoManager );

    try
    {
        auto aDialogInput = std::make_shared<TitleDialogData>();
        aDialogInput->readFromModel( getChartModel() );

        SolarMutexGuard aGuard;
        auto aDlg = std::make_shared<SchTitleDlg>(GetChartFrame(), *aDialogInput);
        weld::DialogController::runAsync(aDlg, [this, aDlg, aDialogInput, xUndoGuard=std::move(xUndoGuard)](int nResult){
            if ( nResult == RET_OK )
            {
                // lock controllers till end of block
                ControllerLockGuardUNO aCLGuard( getChartModel() );
                TitleDialogData aDialogOutput( impl_createReferenceSizeProvider() );
                aDlg->getResult( aDialogOutput );
                bool bChanged = aDialogOutput.writeDifferenceToModel( getChartModel(), m_xCC, aDialogInput.get() );
                if( bChanged )
                    xUndoGuard->commit();
            }
        });
    }
    catch(const uno::RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

void ChartController::executeDispatch_DeleteLegend()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Delete, SchResId( STR_OBJECT_LEGEND )),
        m_xUndoManager );

    LegendHelper::hideLegend(*getChartModel());
    aUndoGuard.commit();
}

void ChartController::executeDispatch_InsertLegend()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Insert, SchResId( STR_OBJECT_LEGEND )),
        m_xUndoManager );

    LegendHelper::showLegend(*getChartModel(), m_xCC);
    aUndoGuard.commit();
}

void ChartController::executeDispatch_OpenLegendDialog()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Insert, SchResId( STR_OBJECT_LEGEND )),
        m_xUndoManager );

    try
    {
        //prepare and open dialog
        SolarMutexGuard aGuard;
        SchLegendDlg aDlg(GetChartFrame(), m_xCC);
        aDlg.init( getChartModel() );
        if (aDlg.run() == RET_OK)
        {
            // lock controllers till end of block
            ControllerLockGuardUNO aCLGuard( getChartModel() );
            aDlg.writeToModel( getChartModel() );
            aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

void ChartController::executeDispatch_InsertMenu_DataLabels()
{
    std::shared_ptr<UndoGuard> aUndoGuard = std::make_shared<UndoGuard>(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Insert, SchResId( STR_OBJECT_DATALABELS )),
        m_xUndoManager );

    //if a series is selected insert labels for that series only:
    rtl::Reference< DataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getChartModel());
    if( xSeries.is() )
    {
        // add labels
        DataSeriesHelper::insertDataLabelsToSeriesAndAllPoints( xSeries );

        OUString aChildParticle( ObjectIdentifier::getStringForType( OBJECTTYPE_DATA_LABELS ) + "=" );
        OUString aObjectCID = ObjectIdentifier::createClassifiedIdentifierForParticles(
            ObjectIdentifier::getSeriesParticleFromCID(m_aSelection.getSelectedCID()), aChildParticle );

        ChartController::executeDlg_ObjectProperties_withUndoGuard( aUndoGuard, aObjectCID, true );
        return;
    }
    try
    {
        wrapper::AllDataLabelItemConverter aItemConverter(
            getChartModel(),
            m_pDrawModelWrapper->GetItemPool(),
            m_pDrawModelWrapper->getSdrModel(),
            getChartModel() );
        SfxItemSet aItemSet = aItemConverter.CreateEmptyItemSet();
        aItemConverter.FillItemSet( aItemSet );

        //prepare and open dialog
        SolarMutexGuard aGuard;

        //get number formatter
        NumberFormatterWrapper aNumberFormatterWrapper( getChartModel() );
        SvNumberFormatter* pNumberFormatter = aNumberFormatterWrapper.getSvNumberFormatter();

        DataLabelsDialog aDlg(GetChartFrame(), aItemSet, pNumberFormatter);

        if (aDlg.run() == RET_OK)
        {
            SfxItemSet aOutItemSet = aItemConverter.CreateEmptyItemSet();
            aDlg.FillItemSet(aOutItemSet);
            // lock controllers till end of block
            ControllerLockGuardUNO aCLGuard( getChartModel() );
            bool bChanged = aItemConverter.ApplyItemSet( aOutItemSet );//model should be changed now
            if( bChanged )
                aUndoGuard->commit();
        }
    }
    catch(const uno::RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

void ChartController::executeDispatch_InsertMeanValue()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Insert, SchResId( STR_OBJECT_AVERAGE_LINE )),
        m_xUndoManager );
    lcl_InsertMeanValueLine( ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(),
                                                                    getChartModel() ) );
    aUndoGuard.commit();
}

void ChartController::executeDispatch_InsertMenu_MeanValues()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Insert, SchResId( STR_OBJECT_AVERAGE_LINE )),
        m_xUndoManager );

    rtl::Reference< DataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getChartModel() );
    if( xSeries.is() )
    {
        //if a series is selected insert mean value only for that series:
        lcl_InsertMeanValueLine( xSeries );
    }
    else if (rtl::Reference<Diagram> xDiagram = getFirstDiagram())
    {
        std::vector< rtl::Reference< DataSeries > > aSeries =
            xDiagram->getDataSeries();

        for( const auto& xSrs : aSeries )
            lcl_InsertMeanValueLine( xSrs );
    }
    aUndoGuard.commit();
}

void ChartController::executeDispatch_InsertMenu_Trendlines()
{
    OUString aCID = m_aSelection.getSelectedCID();

    rtl::Reference< DataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID( aCID, getChartModel() );

    if( !xSeries.is() )
        return;

    executeDispatch_InsertTrendline();
}

void ChartController::executeDispatch_InsertTrendline()
{
    rtl::Reference< DataSeries > xRegressionCurveContainer =
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getChartModel());

    if( !xRegressionCurveContainer.is() )
        return;

    auto xUndoGuard = std::make_shared<UndoLiveUpdateGuard>(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Insert, SchResId( STR_OBJECT_CURVE )),
        m_xUndoManager );

    rtl::Reference< RegressionCurveModel > xCurve =
        RegressionCurveHelper::addRegressionCurve(
            SvxChartRegress::Linear,
            xRegressionCurveContainer );

    if( !xCurve.is())
        return;

    auto aItemConverter = std::make_shared<wrapper::RegressionCurveItemConverter>(
        xCurve, xRegressionCurveContainer, m_pDrawModelWrapper->getSdrModel().GetItemPool(),
        m_pDrawModelWrapper->getSdrModel(),
        getChartModel() );

    // open dialog
    SfxItemSet aItemSet = aItemConverter->CreateEmptyItemSet();
    aItemConverter->FillItemSet( aItemSet );
    ObjectPropertiesDialogParameter aDialogParameter(
        ObjectIdentifier::createDataCurveCID(
            ObjectIdentifier::getSeriesParticleFromCID( m_aSelection.getSelectedCID()),
            RegressionCurveHelper::getRegressionCurveIndex( xRegressionCurveContainer, xCurve ), false ));
    aDialogParameter.init( getChartModel() );
    ViewElementListProvider aViewElementListProvider( m_pDrawModelWrapper.get());
    SolarMutexGuard aGuard;
    auto aDialog = std::make_shared<SchAttribTabDlg>(GetChartFrame(), &aItemSet, &aDialogParameter,
                                                     &aViewElementListProvider, getChartModel());

    // note: when a user pressed "OK" but didn't change any settings in the
    // dialog, the SfxTabDialog returns "Cancel"
    SfxTabDialogController::runAsync(aDialog, [this, aDialog, aItemConverter, xUndoGuard=std::move(xUndoGuard)](int nResult) {
        if ( nResult == RET_OK || aDialog->DialogWasClosedWithOK() )
        {
            const SfxItemSet* pOutItemSet = aDialog->GetOutputItemSet();
            if( pOutItemSet )
            {
                ControllerLockGuardUNO aCLGuard( getChartModel() );
                aItemConverter->ApplyItemSet( *pOutItemSet );
            }
            xUndoGuard->commit();
        }
    });
}

void ChartController::executeDispatch_InsertErrorBars( bool bYError )
{
    ObjectType objType = bYError ? OBJECTTYPE_DATA_ERRORS_Y : OBJECTTYPE_DATA_ERRORS_X;

    //if a series is selected insert error bars for that series only:
    rtl::Reference< DataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getChartModel() );

    if( xSeries.is())
    {
        auto xUndoGuard = std::make_shared<UndoLiveUpdateGuard>(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::ActionType::Insert,
                SchResId( bYError ? STR_OBJECT_ERROR_BARS_Y : STR_OBJECT_ERROR_BARS_X )),
            m_xUndoManager );

        // add error bars with standard deviation
        uno::Reference< beans::XPropertySet > xErrorBarProp(
            StatisticsHelper::addErrorBars( xSeries,
                                            css::chart::ErrorBarStyle::STANDARD_DEVIATION,
                                            bYError));

        // get an appropriate item converter
        auto aItemConverter = std::make_shared<wrapper::ErrorBarItemConverter> (
            getChartModel(), xErrorBarProp, m_pDrawModelWrapper->getSdrModel().GetItemPool(),
            m_pDrawModelWrapper->getSdrModel(),
            getChartModel() );

        // open dialog
        SfxItemSet aItemSet = aItemConverter->CreateEmptyItemSet();
        aItemSet.Put(SfxBoolItem(SCHATTR_STAT_ERRORBAR_TYPE,bYError));
        aItemConverter->FillItemSet( aItemSet );
        ObjectPropertiesDialogParameter aDialogParameter(
            ObjectIdentifier::createClassifiedIdentifierWithParent(
                objType, u"", m_aSelection.getSelectedCID()));
        aDialogParameter.init( getChartModel() );
        ViewElementListProvider aViewElementListProvider( m_pDrawModelWrapper.get());
        SolarMutexGuard aGuard;
        auto aDlg = std::make_shared<SchAttribTabDlg>(
                GetChartFrame(), &aItemSet, &aDialogParameter,
                &aViewElementListProvider,
                getChartModel() );
        aDlg->SetAxisMinorStepWidthForErrorBarDecimals(
            InsertErrorBarsDialog::getAxisMinorStepWidthForErrorBarDecimals( getChartModel(),
                                                                             m_xChartView, m_aSelection.getSelectedCID()));

        // note: when a user pressed "OK" but didn't change any settings in the
        // dialog, the SfxTabDialog returns "Cancel"
        SfxTabDialogController::runAsync(aDlg, [this, aDlg, aItemConverter, xUndoGuard=std::move(xUndoGuard)](int nResult) {
            if ( nResult == RET_OK || aDlg->DialogWasClosedWithOK() )
            {
                const SfxItemSet* pOutItemSet = aDlg->GetOutputItemSet();
                if( pOutItemSet )
                {
                    ControllerLockGuardUNO aCLGuard( getChartModel() );
                    aItemConverter->ApplyItemSet( *pOutItemSet );
                }
                xUndoGuard->commit();
            }
        });
    }
    else
    {
        //if no series is selected insert error bars for all series
        auto xUndoGuard = std::make_shared<UndoGuard>(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::ActionType::Insert,
                ObjectNameProvider::getName_ObjectForAllSeries( objType ) ),
            m_xUndoManager );

        try
        {
            auto aItemConverter = std::make_shared<wrapper::AllSeriesStatisticsConverter>(
                getChartModel(), m_pDrawModelWrapper->GetItemPool() );
            SfxItemSet aItemSet = aItemConverter->CreateEmptyItemSet();
            aItemConverter->FillItemSet( aItemSet );

            //prepare and open dialog
            SolarMutexGuard aGuard;
            auto aDlg = std::make_shared<InsertErrorBarsDialog>(
                GetChartFrame(), aItemSet,
                getChartModel(),
                bYError ? ErrorBarResources::ERROR_BAR_Y : ErrorBarResources::ERROR_BAR_X);

            aDlg->SetAxisMinorStepWidthForErrorBarDecimals(
                InsertErrorBarsDialog::getAxisMinorStepWidthForErrorBarDecimals( getChartModel(), m_xChartView, u"" ) );

            weld::DialogController::runAsync(aDlg, [this, aDlg, aItemConverter, xUndoGuard=std::move(xUndoGuard)](int nResult) {
                if ( nResult == RET_OK )
                {
                    SfxItemSet aOutItemSet = aItemConverter->CreateEmptyItemSet();
                    aDlg->FillItemSet( aOutItemSet );

                    // lock controllers till end of block
                    ControllerLockGuardUNO aCLGuard( getChartModel() );
                    bool bChanged = aItemConverter->ApplyItemSet( aOutItemSet );//model should be changed now
                    if( bChanged )
                        xUndoGuard->commit();
                }
            });
        }
        catch(const uno::RuntimeException&)
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
}

void ChartController::executeDispatch_InsertTrendlineEquation( bool bInsertR2 )
{
    uno::Reference< chart2::XRegressionCurve > xRegCurve(
        ObjectIdentifier::getObjectPropertySet( m_aSelection.getSelectedCID(), getChartModel() ), uno::UNO_QUERY );
    if( !xRegCurve.is() )
    {
        rtl::Reference< DataSeries > xRegCurveCnt =
            ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getChartModel() );
        xRegCurve.set( RegressionCurveHelper::getFirstCurveNotMeanValueLine( xRegCurveCnt ) );
    }
    if( !xRegCurve.is())
        return;

    uno::Reference< beans::XPropertySet > xEqProp( xRegCurve->getEquationProperties());
    if( xEqProp.is())
    {
        UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::ActionType::Insert, SchResId( STR_OBJECT_CURVE_EQUATION )),
            m_xUndoManager );
        xEqProp->setPropertyValue( u"ShowEquation"_ustr, uno::Any( true ));
        xEqProp->setPropertyValue( u"XName"_ustr, uno::Any( u"x"_ustr ));
        xEqProp->setPropertyValue( u"YName"_ustr, uno::Any( u"f(x)"_ustr ));
        xEqProp->setPropertyValue( u"ShowCorrelationCoefficient"_ustr, uno::Any( bInsertR2 ));
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_InsertR2Value()
{
    uno::Reference< beans::XPropertySet > xEqProp =
        ObjectIdentifier::getObjectPropertySet( m_aSelection.getSelectedCID(), getChartModel() );
    if( xEqProp.is())
    {
        UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::ActionType::Insert, SchResId( STR_OBJECT_CURVE_EQUATION )),
            m_xUndoManager );
        xEqProp->setPropertyValue( u"ShowCorrelationCoefficient"_ustr, uno::Any( true ));
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_DeleteR2Value()
{
    uno::Reference< beans::XPropertySet > xEqProp =
        ObjectIdentifier::getObjectPropertySet( m_aSelection.getSelectedCID(), getChartModel() );
    if( xEqProp.is())
    {
        UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::ActionType::Insert, SchResId( STR_OBJECT_CURVE_EQUATION )),
            m_xUndoManager );
        xEqProp->setPropertyValue( u"ShowCorrelationCoefficient"_ustr, uno::Any( false ));
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_DeleteMeanValue()
{
    rtl::Reference< DataSeries > xRegCurveCnt =
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getChartModel() );
    if( xRegCurveCnt.is())
    {
        UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::ActionType::Delete, SchResId( STR_OBJECT_AVERAGE_LINE )),
            m_xUndoManager );
        RegressionCurveHelper::removeMeanValueLine( xRegCurveCnt );
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_DeleteTrendline()
{
    rtl::Reference< DataSeries > xRegCurveCnt =
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getChartModel() );
    if( xRegCurveCnt.is())
    {
        UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::ActionType::Delete, SchResId( STR_OBJECT_CURVE )),
            m_xUndoManager );
        RegressionCurveHelper::removeAllExceptMeanValueLine( xRegCurveCnt );
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_DeleteTrendlineEquation()
{
    rtl::Reference< DataSeries > xRegCurveCnt =
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getChartModel() );
    if( xRegCurveCnt.is())
    {
        UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::ActionType::Delete, SchResId( STR_OBJECT_CURVE_EQUATION )),
            m_xUndoManager );
        RegressionCurveHelper::removeEquations( xRegCurveCnt );
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_DeleteErrorBars( bool bYError )
{
    rtl::Reference< DataSeries > xDataSeries =
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getChartModel() );
    if( xDataSeries.is())
    {
        UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::ActionType::Delete, SchResId( STR_OBJECT_CURVE )),
            m_xUndoManager );
        StatisticsHelper::removeErrorBars( xDataSeries, bYError );
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_InsertDataLabels()
{
    rtl::Reference< DataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getChartModel() );
    if( xSeries.is() )
    {
        UndoGuard aUndoGuard( ActionDescriptionProvider::createDescription( ActionDescriptionProvider::ActionType::Insert,
            SchResId( STR_OBJECT_DATALABELS )),
            m_xUndoManager );
        DataSeriesHelper::insertDataLabelsToSeriesAndAllPoints( xSeries );
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_InsertDataLabel()
{
    UndoGuard aUndoGuard( ActionDescriptionProvider::createDescription( ActionDescriptionProvider::ActionType::Insert,
        SchResId( STR_OBJECT_LABEL )),
        m_xUndoManager );
    DataSeriesHelper::insertDataLabelToPoint( ObjectIdentifier::getObjectPropertySet( m_aSelection.getSelectedCID(), getChartModel() ) );
    aUndoGuard.commit();
}

void ChartController::executeDispatch_DeleteDataLabels()
{
    rtl::Reference< DataSeries > xSeries =
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getChartModel() );
    if( xSeries.is() )
    {
        UndoGuard aUndoGuard( ActionDescriptionProvider::createDescription( ActionDescriptionProvider::ActionType::Delete,
            SchResId( STR_OBJECT_DATALABELS )),
            m_xUndoManager );
        DataSeriesHelper::deleteDataLabelsFromSeriesAndAllPoints( xSeries );
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_DeleteDataLabel()
{
    UndoGuard aUndoGuard( ActionDescriptionProvider::createDescription( ActionDescriptionProvider::ActionType::Delete,
        SchResId( STR_OBJECT_LABEL )),
        m_xUndoManager );
    DataSeriesHelper::deleteDataLabelsFromPoint( ObjectIdentifier::getObjectPropertySet( m_aSelection.getSelectedCID(), getChartModel() ) );
    aUndoGuard.commit();
}

void ChartController::executeDispatch_ResetAllDataPoints()
{
    UndoGuard aUndoGuard( ActionDescriptionProvider::createDescription( ActionDescriptionProvider::ActionType::Format,
        SchResId( STR_OBJECT_DATAPOINTS )),
        m_xUndoManager );
    rtl::Reference< DataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getChartModel() );
    if( xSeries.is() )
        xSeries->resetAllDataPoints();
    aUndoGuard.commit();
}
void ChartController::executeDispatch_ResetDataPoint()
{
    UndoGuard aUndoGuard( ActionDescriptionProvider::createDescription( ActionDescriptionProvider::ActionType::Format,
        SchResId( STR_OBJECT_DATAPOINT )),
        m_xUndoManager );
    rtl::Reference< DataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getChartModel() );
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
        rtl::Reference< Title > xTitle;
        {
            UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::ActionType::Insert, SchResId( STR_OBJECT_TITLE )),
            m_xUndoManager );

            rtl::Reference< Axis > xAxis = ObjectIdentifier::getAxisForCID( m_aSelection.getSelectedCID(), getChartModel() );
            sal_Int32 nDimensionIndex = -1;
            sal_Int32 nCooSysIndex = -1;
            sal_Int32 nAxisIndex = -1;
            AxisHelper::getIndicesForAxis( xAxis, getFirstDiagram(), nCooSysIndex, nDimensionIndex, nAxisIndex );

            TitleHelper::eTitleType eTitleType = TitleHelper::X_AXIS_TITLE;
            if( nDimensionIndex==0 )
                eTitleType = nAxisIndex==0 ? TitleHelper::X_AXIS_TITLE : TitleHelper::SECONDARY_X_AXIS_TITLE;
            else if( nDimensionIndex==1 )
                eTitleType = nAxisIndex==0 ? TitleHelper::Y_AXIS_TITLE : TitleHelper::SECONDARY_Y_AXIS_TITLE;
            else
                eTitleType = TitleHelper::Z_AXIS_TITLE;

            ReferenceSizeProvider aRefSizeProvider( impl_createReferenceSizeProvider());
            xTitle = TitleHelper::createTitle( eTitleType, ObjectNameProvider::getTitleNameByType(eTitleType), getChartModel(), m_xCC, &aRefSizeProvider );
            aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

void ChartController::executeDispatch_InsertAxis()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Insert, SchResId( STR_OBJECT_AXIS )),
        m_xUndoManager );

    try
    {
        rtl::Reference< Axis > xAxis = ObjectIdentifier::getAxisForCID( m_aSelection.getSelectedCID(), getChartModel() );
        if( xAxis.is() )
        {
            AxisHelper::makeAxisVisible( xAxis );
            aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

void ChartController::executeDispatch_DeleteAxis()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Delete, SchResId( STR_OBJECT_AXIS )),
        m_xUndoManager );

    try
    {
        rtl::Reference< Axis > xAxis = ObjectIdentifier::getAxisForCID( m_aSelection.getSelectedCID(), getChartModel() );
        if( xAxis.is() )
        {
            AxisHelper::makeAxisInvisible( xAxis );
            aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

void ChartController::executeDispatch_InsertMajorGrid()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Insert, SchResId( STR_OBJECT_GRID )),
        m_xUndoManager );

    try
    {
        rtl::Reference< Axis > xAxis = ObjectIdentifier::getAxisForCID( m_aSelection.getSelectedCID(), getChartModel() );
        if( xAxis.is() )
        {
            AxisHelper::makeGridVisible( xAxis->getGridProperties2() );
            aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

void ChartController::executeDispatch_DeleteMajorGrid()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Delete, SchResId( STR_OBJECT_GRID )),
        m_xUndoManager );

    try
    {
        rtl::Reference< Axis > xAxis = ObjectIdentifier::getAxisForCID( m_aSelection.getSelectedCID(), getChartModel() );
        if( xAxis.is() )
        {
            AxisHelper::makeGridInvisible( xAxis->getGridProperties2() );
            aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

void ChartController::executeDispatch_InsertMinorGrid()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Insert, SchResId( STR_OBJECT_GRID )),
        m_xUndoManager );

    try
    {
        rtl::Reference< Axis > xAxis = ObjectIdentifier::getAxisForCID( m_aSelection.getSelectedCID(), getChartModel() );
        if( xAxis.is() )
        {
            std::vector< rtl::Reference< ::chart::GridProperties > > aSubGrids( xAxis->getSubGridProperties2() );
            for( rtl::Reference< GridProperties > const & props : aSubGrids)
                AxisHelper::makeGridVisible( props );
            aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

void ChartController::executeDispatch_DeleteMinorGrid()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::ActionType::Delete, SchResId(STR_OBJECT_GRID)),
        m_xUndoManager );

    try
    {
        rtl::Reference< Axis > xAxis = ObjectIdentifier::getAxisForCID( m_aSelection.getSelectedCID(), getChartModel() );
        if( xAxis.is() )
        {
            std::vector< rtl::Reference< ::chart::GridProperties > > aSubGrids( xAxis->getSubGridProperties2() );
            for( rtl::Reference< ::chart::GridProperties > const & props : aSubGrids)
                AxisHelper::makeGridInvisible( props );
            aUndoGuard.commit();
        }
    }
    catch(const uno::RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
