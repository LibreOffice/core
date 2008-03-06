/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartController_Insert.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:57:00 $
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
#include "ChartController.hxx"

#include "dlg_InsertAxis_Grid.hxx"
#include "dlg_InsertDataLabel.hxx"
#include "dlg_InsertLegend.hxx"
#include "dlg_InsertTrendline.hxx"
#include "dlg_InsertErrorBars.hxx"
#include "dlg_InsertTitle.hxx"
#include "dlg_ObjectProperties.hxx"

#include "ChartWindow.hxx"
#include "ChartModelHelper.hxx"
#include "AxisHelper.hxx"
#include "TitleHelper.hxx"
#include "DiagramHelper.hxx"
#include "ContextHelper.hxx"
#include "macros.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "chartview/NumberFormatterWrapper.hxx"
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

#include <com/sun/star/chart2/XRegressionCurve.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>

#ifndef _SVX_ACTIONDESCRIPTIONPROVIDER_HXX
#include <svx/ActionDescriptionProvider.hxx>
#endif

//--------------------------------------

// header for define RET_OK
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
// header for class OUStringBuffer
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
// header for class Application
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
// header for class ::vos::OGuard
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif


using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

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

void SAL_CALL ChartController::executeDispatch_InsertAxis()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, ::rtl::OUString( String( SchResId( STR_OBJECT_AXES )))),
        m_xUndoManager, m_aModel->getModel() );

    try
    {
        InsertAxisOrGridDialogData aDialogInput;
        uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram(m_aModel->getModel());
        AxisHelper::getAxisOrGridExcistence( aDialogInput.aExistenceList, xDiagram, sal_True );
        AxisHelper::getAxisOrGridPossibilities( aDialogInput.aPossibilityList, xDiagram, sal_True );

        ::vos::OGuard aGuard( Application::GetSolarMutex());
        SchAxisDlg aDlg( m_pChartWindow, aDialogInput );
        if( aDlg.Execute() == RET_OK )
        {
            // lock controllers till end of block
            ControllerLockGuard aCLGuard( m_aModel->getModel());

            InsertAxisOrGridDialogData aDialogOutput;
            aDlg.getResult( aDialogOutput );
            ::std::auto_ptr< ReferenceSizeProvider > mpRefSizeProvider(
                impl_createReferenceSizeProvider());
            bool bChanged = AxisHelper::changeVisibilityOfAxes( xDiagram
                , aDialogInput.aExistenceList, aDialogOutput.aExistenceList, m_xCC
                , mpRefSizeProvider.get() );
            if( bChanged )
                aUndoGuard.commitAction();
        }
    }
    catch( uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void SAL_CALL ChartController::executeDispatch_InsertGrid()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, ::rtl::OUString( String( SchResId( STR_OBJECT_GRIDS )))),
        m_xUndoManager, m_aModel->getModel() );

    try
    {
        InsertAxisOrGridDialogData aDialogInput;
        uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram(m_aModel->getModel());
        AxisHelper::getAxisOrGridExcistence( aDialogInput.aExistenceList, xDiagram, sal_False );
        AxisHelper::getAxisOrGridPossibilities( aDialogInput.aPossibilityList, xDiagram, sal_False );

        ::vos::OGuard aGuard( Application::GetSolarMutex());
        SchGridDlg aDlg( m_pChartWindow, aDialogInput );//aItemSet, b3D, bNet, bSecondaryX, bSecondaryY );
        if( aDlg.Execute() == RET_OK )
        {
            // lock controllers till end of block
            ControllerLockGuard aCLGuard( m_aModel->getModel());
            InsertAxisOrGridDialogData aDialogOutput;
            aDlg.getResult( aDialogOutput );
            bool bChanged = AxisHelper::changeVisibilityOfGrids( xDiagram
                , aDialogInput.aExistenceList, aDialogOutput.aExistenceList, m_xCC );
            if( bChanged )
                aUndoGuard.commitAction();
        }
    }
    catch( uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void SAL_CALL ChartController::executeDispatch_InsertTitle()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, ::rtl::OUString( String( SchResId( STR_OBJECT_TITLES )))),
        m_xUndoManager, m_aModel->getModel() );

    try
    {
        TitleDialogData aDialogInput;
        aDialogInput.readFromModel( m_aModel->getModel() );

        ::vos::OGuard aGuard( Application::GetSolarMutex());
        SchTitleDlg aDlg( m_pChartWindow, aDialogInput );
        if( aDlg.Execute() == RET_OK )
        {
            // lock controllers till end of block
            ControllerLockGuard aCLGuard( m_aModel->getModel());
            TitleDialogData aDialogOutput( impl_createReferenceSizeProvider());
            aDlg.getResult( aDialogOutput );
            bool bChanged = aDialogOutput.writeDifferenceToModel( m_aModel->getModel(), m_xCC, &aDialogInput );
            if( bChanged )
                aUndoGuard.commitAction();
        }
    }
    catch( uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void SAL_CALL ChartController::executeDispatch_InsertLegend()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, ::rtl::OUString( String( SchResId( STR_OBJECT_LEGEND )))),
        m_xUndoManager, m_aModel->getModel() );

    try
    {
        //prepare and open dialog
        ::vos::OGuard aGuard( Application::GetSolarMutex());
        SchLegendDlg aDlg( m_pChartWindow, m_xCC );
        aDlg.init( m_aModel->getModel() );
        if( aDlg.Execute() == RET_OK )
        {
            // lock controllers till end of block
            ControllerLockGuard aCLGuard( m_aModel->getModel() );
            bool bChanged = aDlg.writeToModel( m_aModel->getModel() );
            if( bChanged )
                aUndoGuard.commitAction();
        }
    }
    catch( uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void SAL_CALL ChartController::executeDispatch_InsertDataLabel()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, ::rtl::OUString( String( SchResId( STR_OBJECT_DATALABELS )))),
        m_xUndoManager, m_aModel->getModel() );

    try
    {
        wrapper::AllDataLabelItemConverter aItemConverter(
            m_aModel->getModel(),
            m_pDrawModelWrapper->GetItemPool(),
            m_pDrawModelWrapper->getSdrModel(),
            uno::Reference< lang::XMultiServiceFactory >( m_aModel->getModel(), uno::UNO_QUERY ));
        SfxItemSet aItemSet = aItemConverter.CreateEmptyItemSet();
        aItemConverter.FillItemSet( aItemSet );

        //prepare and open dialog
        ::vos::OGuard aGuard( Application::GetSolarMutex());

        //get number formatter
        uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( m_aModel->getModel(), uno::UNO_QUERY );
        NumberFormatterWrapper aNumberFormatterWrapper( xNumberFormatsSupplier );
        SvNumberFormatter* pNumberFormatter = aNumberFormatterWrapper.getSvNumberFormatter();

        DataLabelsDialog aDlg( m_pChartWindow, aItemSet, pNumberFormatter);

        if( aDlg.Execute() == RET_OK )
        {
            SfxItemSet aOutItemSet = aItemConverter.CreateEmptyItemSet();
            aDlg.FillItemSet( aOutItemSet );
            // lock controllers till end of block
            ControllerLockGuard aCLGuard( m_aModel->getModel());
            bool bChanged = aItemConverter.ApplyItemSet( aOutItemSet );//model should be changed now
            if( bChanged )
                aUndoGuard.commitAction();
        }
    }
    catch( uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void SAL_CALL ChartController::executeDispatch_InsertYErrorbars()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, ::rtl::OUString( String( SchResId( STR_PAGE_YERROR_BARS )))),
        m_xUndoManager, m_aModel->getModel() );

    try
    {
        wrapper::AllSeriesStatisticsConverter aItemConverter(
            m_aModel->getModel(), m_pDrawModelWrapper->GetItemPool() );
        SfxItemSet aItemSet = aItemConverter.CreateEmptyItemSet();
        aItemConverter.FillItemSet( aItemSet );

        //prepare and open dialog
        ::vos::OGuard aGuard( Application::GetSolarMutex());
        InsertErrorBarsDialog aDlg(
            m_pChartWindow, aItemSet,
            uno::Reference< chart2::XChartDocument >( m_aModel->getModel(), uno::UNO_QUERY ));
        aDlg.SetAxisMinorStepWidthForErrorBarDecimals(
            InsertErrorBarsDialog::getAxisMinorStepWidthForErrorBarDecimals( m_aModel->getModel(), m_xChartView, rtl::OUString() ) );

        if( aDlg.Execute() == RET_OK )
        {
            SfxItemSet aOutItemSet = aItemConverter.CreateEmptyItemSet();
            aDlg.FillItemSet( aOutItemSet );

            // lock controllers till end of block
            ControllerLockGuard aCLGuard( m_aModel->getModel());
            bool bChanged = aItemConverter.ApplyItemSet( aOutItemSet );//model should be changed now
            if( bChanged )
                aUndoGuard.commitAction();
        }
    }
    catch( uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void SAL_CALL ChartController::executeDispatch_InsertMeanValue()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, ::rtl::OUString( String( SchResId( STR_OBJECT_AVERAGE_LINE )))),
        m_xUndoManager, m_aModel->getModel());
    lcl_InsertMeanValueLine( m_xCC ).operator()(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), m_aModel->getModel()));
    aUndoGuard.commitAction();
}

void SAL_CALL ChartController::executeDispatch_InsertMeanValues()
{
    ::std::vector< uno::Reference< chart2::XDataSeries > > aSeries(
        DiagramHelper::getDataSeriesFromDiagram( ChartModelHelper::findDiagram( m_aModel->getModel())));
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, ::rtl::OUString( String( SchResId( STR_OBJECT_AVERAGE_LINE )))),
        m_xUndoManager, m_aModel->getModel() );
    ::std::for_each( aSeries.begin(), aSeries.end(), lcl_InsertMeanValueLine( m_xCC ));
    aUndoGuard.commitAction();
}

void SAL_CALL ChartController::executeDispatch_InsertTrendlines()
{
    UndoGuard aUndoGuard(
        ActionDescriptionProvider::createDescription(
            ActionDescriptionProvider::INSERT, ::rtl::OUString( String( SchResId( STR_OBJECT_CURVES )))),
        m_xUndoManager, m_aModel->getModel() );

    try
    {
        wrapper::AllSeriesStatisticsConverter aItemConverter(
            m_aModel->getModel(), m_pDrawModelWrapper->GetItemPool() );
        SfxItemSet aItemSet = aItemConverter.CreateEmptyItemSet();
        aItemConverter.FillItemSet( aItemSet );

        //prepare and open dialog
        ::vos::OGuard aGuard( Application::GetSolarMutex());
        InsertTrendlineDialog aDlg( m_pChartWindow, aItemSet );
        aDlg.adjustSize();

        if( aDlg.Execute() == RET_OK )
        {
            SfxItemSet aOutItemSet = aItemConverter.CreateEmptyItemSet();
            aDlg.FillItemSet( aOutItemSet );

            // lock controllers till end of block
            ControllerLockGuard aCLGuard( m_aModel->getModel());
            bool bChanged = aItemConverter.ApplyItemSet( aOutItemSet );//model should be changed now
            if( bChanged )
                aUndoGuard.commitAction();
        }
    }
    catch( uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void SAL_CALL ChartController::executeDispatch_InsertTrendline()
{
    uno::Reference< chart2::XRegressionCurveContainer > xRegCurveCnt(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), m_aModel->getModel()), uno::UNO_QUERY );
    if( xRegCurveCnt.is())
    {
        UndoLiveUpdateGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::INSERT, ::rtl::OUString( String( SchResId( STR_OBJECT_CURVE )))),
            m_xUndoManager, m_aModel->getModel() );

        // add a linear curve
        RegressionCurveHelper::addRegressionCurve(
            RegressionCurveHelper::REGRESSION_TYPE_LINEAR, xRegCurveCnt, m_xCC );

        // get an appropriate item converter
        uno::Reference< chart2::XRegressionCurve > xCurve(
            RegressionCurveHelper::getFirstCurveNotMeanValueLine( xRegCurveCnt ));
        uno::Reference< beans::XPropertySet > xCurveProp( xCurve, uno::UNO_QUERY );
        if( !xCurveProp.is())
            return;
        wrapper::RegressionCurveItemConverter aItemConverter(
            xCurveProp, xRegCurveCnt, m_pDrawModelWrapper->getSdrModel().GetItemPool(),
            m_pDrawModelWrapper->getSdrModel(),
            uno::Reference< lang::XMultiServiceFactory >( m_aModel->getModel(), uno::UNO_QUERY ));

        // open dialog
        SfxItemSet aItemSet = aItemConverter.CreateEmptyItemSet();
        aItemConverter.FillItemSet( aItemSet );
        ObjectPropertiesDialogParameter aDialogParameter = ObjectPropertiesDialogParameter(
            ObjectIdentifier::createDataCurveCID(
                ObjectIdentifier::getSeriesParticleFromCID( m_aSelection.getSelectedCID()),
                RegressionCurveHelper::getRegressionCurveIndex( xRegCurveCnt, xCurve ), false ));
        aDialogParameter.init( m_aModel->getModel() );
        ViewElementListProvider aViewElementListProvider( m_pDrawModelWrapper.get());
        ::vos::OGuard aGuard( Application::GetSolarMutex());
        SchAttribTabDlg aDlg( m_pChartWindow, &aItemSet, &aDialogParameter, &aViewElementListProvider,
                              uno::Reference< util::XNumberFormatsSupplier >( m_aModel->getModel(), uno::UNO_QUERY ));

        // note: when a user pressed "OK" but didn't change any settings in the
        // dialog, the SfxTabDialog returns "Cancel"
        if( aDlg.Execute() == RET_OK || aDlg.DialogWasClosedWithOK())
        {
            const SfxItemSet* pOutItemSet = aDlg.GetOutputItemSet();
            if( pOutItemSet )
            {
                ControllerLockGuard aCLGuard( m_aModel->getModel());
                aItemConverter.ApplyItemSet( *pOutItemSet );
            }
            aUndoGuard.commitAction();
        }
    }
}

void SAL_CALL ChartController::executeDispatch_InsertYErrorbar()
{
    uno::Reference< chart2::XDataSeries > xSeries(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), m_aModel->getModel()), uno::UNO_QUERY );
    if( xSeries.is())
    {
        UndoLiveUpdateGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::INSERT, ::rtl::OUString( String( SchResId( STR_OBJECT_ERROR_INDICATOR )))),
            m_xUndoManager, m_aModel->getModel() );

        // add error bars with standard deviation
        uno::Reference< beans::XPropertySet > xErrorBarProp(
            StatisticsHelper::addErrorBars( xSeries, m_xCC, ::com::sun::star::chart::ErrorBarStyle::STANDARD_DEVIATION ));

        // get an appropriate item converter
        wrapper::ErrorBarItemConverter aItemConverter(
            m_aModel->getModel(), xErrorBarProp, m_pDrawModelWrapper->getSdrModel().GetItemPool(),
            m_pDrawModelWrapper->getSdrModel(),
            uno::Reference< lang::XMultiServiceFactory >( m_aModel->getModel(), uno::UNO_QUERY ));

        // open dialog
        SfxItemSet aItemSet = aItemConverter.CreateEmptyItemSet();
        aItemConverter.FillItemSet( aItemSet );
        ObjectPropertiesDialogParameter aDialogParameter = ObjectPropertiesDialogParameter(
            ObjectIdentifier::createClassifiedIdentifierWithParent(
                OBJECTTYPE_DATA_ERRORS, ::rtl::OUString(), m_aSelection.getSelectedCID()));
        aDialogParameter.init( m_aModel->getModel() );
        ViewElementListProvider aViewElementListProvider( m_pDrawModelWrapper.get());
        ::vos::OGuard aGuard( Application::GetSolarMutex());
        SchAttribTabDlg aDlg( m_pChartWindow, &aItemSet, &aDialogParameter, &aViewElementListProvider,
                              uno::Reference< util::XNumberFormatsSupplier >( m_aModel->getModel(), uno::UNO_QUERY ));
        aDlg.SetAxisMinorStepWidthForErrorBarDecimals(
            InsertErrorBarsDialog::getAxisMinorStepWidthForErrorBarDecimals( m_aModel->getModel(), m_xChartView, m_aSelection.getSelectedCID()));

        // note: when a user pressed "OK" but didn't change any settings in the
        // dialog, the SfxTabDialog returns "Cancel"
        if( aDlg.Execute() == RET_OK || aDlg.DialogWasClosedWithOK())
        {
            const SfxItemSet* pOutItemSet = aDlg.GetOutputItemSet();
            if( pOutItemSet )
            {
                ControllerLockGuard aCLGuard( m_aModel->getModel());
                aItemConverter.ApplyItemSet( *pOutItemSet );
            }
            aUndoGuard.commitAction();
        }
    }
}

void SAL_CALL ChartController::executeDispatch_InsertTrendlineEquation()
{
    uno::Reference< chart2::XRegressionCurve > xRegCurve(
        ObjectIdentifier::getObjectPropertySet( m_aSelection.getSelectedCID(), m_aModel->getModel()), uno::UNO_QUERY );
    if( xRegCurve.is())
    {
        uno::Reference< beans::XPropertySet > xEqProp( xRegCurve->getEquationProperties());
        if( xEqProp.is())
        {
            // using assignment for broken gcc 3.3
            UndoGuard aUndoGuard = UndoGuard(
                ActionDescriptionProvider::createDescription(
                    ActionDescriptionProvider::INSERT, ::rtl::OUString( String( SchResId( STR_OBJECT_CURVE_EQUATION )))),
                m_xUndoManager, m_aModel->getModel() );
            xEqProp->setPropertyValue( C2U("ShowEquation"), uno::makeAny( true ));
            xEqProp->setPropertyValue( C2U("ShowCorrelationCoefficient"), uno::makeAny( false ));
            aUndoGuard.commitAction();
        }
    }
}

void SAL_CALL ChartController::executeDispatch_DeleteMeanValue()
{
    uno::Reference< chart2::XRegressionCurveContainer > xRegCurveCnt(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), m_aModel->getModel()), uno::UNO_QUERY );
    if( xRegCurveCnt.is())
    {
        UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::DELETE, ::rtl::OUString( String( SchResId( STR_OBJECT_AVERAGE_LINE )))),
            m_xUndoManager, m_aModel->getModel());
        RegressionCurveHelper::removeMeanValueLine( xRegCurveCnt );
        aUndoGuard.commitAction();
    }
}

void SAL_CALL ChartController::executeDispatch_DeleteTrendline()
{
    uno::Reference< chart2::XRegressionCurveContainer > xRegCurveCnt(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), m_aModel->getModel()), uno::UNO_QUERY );
    if( xRegCurveCnt.is())
    {
        UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::DELETE, ::rtl::OUString( String( SchResId( STR_OBJECT_CURVE )))),
            m_xUndoManager, m_aModel->getModel());
        RegressionCurveHelper::removeAllExceptMeanValueLine( xRegCurveCnt );
        aUndoGuard.commitAction();
    }
}

void SAL_CALL ChartController::executeDispatch_DeleteYErrorbar()
{
    uno::Reference< chart2::XDataSeries > xDataSeries(
        ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), m_aModel->getModel()));
    if( xDataSeries.is())
    {
        UndoGuard aUndoGuard(
            ActionDescriptionProvider::createDescription(
                ActionDescriptionProvider::DELETE, ::rtl::OUString( String( SchResId( STR_OBJECT_CURVE )))),
            m_xUndoManager, m_aModel->getModel());
        StatisticsHelper::removeErrorBars( xDataSeries );
        aUndoGuard.commitAction();
    }
}


//.............................................................................
} //namespace chart
//.............................................................................
