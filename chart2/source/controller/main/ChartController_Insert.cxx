/*************************************************************************
 *
 *  $RCSfile: ChartController_Insert.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: bm $ $Date: 2003-11-04 12:37:19 $
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
#include "ChartController.hxx"

#include "dlg_InsertAxis_Grid.hxx"
#include "dlg_InsertDataLabel.hxx"
#include "dlg_InsertLegend.hxx"
#include "dlg_InsertStatistic.hxx"
#include "dlg_InsertTitle.hxx"

#include "ChartModelHelper.hxx"
#include "MeterHelper.hxx"
#include "TitleHelper.hxx"
#include "ContextHelper.hxx"
#include "macros.hxx"
#include "DrawModelWrapper.hxx"
#include "MultipleChartConverters.hxx"
#include "LegendItemConverter.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XAXISCONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XAxisContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XBOUNDEDCOORDINATESYSTEMCONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XBoundedCoordinateSystemContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <drafts/com/sun/star/chart2/XDiagram.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XGRIDCONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XGridContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <drafts/com/sun/star/chart2/XTitled.hpp>
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

#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif


//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

namespace
{
//-----------------------------------------------------------------------------
//status of grid or axis

void getExistingAxes( uno::Sequence< sal_Bool >& rExistenceList, const uno::Reference< XDiagram >& xDiagram )
{
    rExistenceList.realloc(6);

    sal_Int32 nN;
    uno::Reference< XAxis > xAxis;
    for(nN=0;nN<3;nN++)
    {
         xAxis = MeterHelper::getAxis( nN, true, xDiagram );
         rExistenceList[nN] = xAxis.is();
    }
    for(nN=3;nN<6;nN++)
    {
         xAxis = MeterHelper::getAxis( nN%3, false, xDiagram );
         rExistenceList[nN] = xAxis.is();
    }
}

void getExistingGrids( uno::Sequence< sal_Bool >& rExistenceList, const uno::Reference< XDiagram >& xDiagram )
{
    sal_Int32 nN;
    uno::Reference< XGrid > xGrid;
    for(nN=0;nN<3;nN++)
    {
        xGrid = MeterHelper::getGrid( nN, true, xDiagram );
        rExistenceList[nN] = xGrid.is();
    }
    for(nN=3;nN<6;nN++)
    {
        xGrid = MeterHelper::getGrid( nN%3, false, xDiagram );
        rExistenceList[nN] = xGrid.is();
    }
}

//-----------------------------------------------------------------------------
//change grid or axis

void createAxis( sal_Int32 nDimensionIndex, bool bMainAxis
                , const uno::Reference< XDiagram >& xDiagram
                , const uno::Reference< uno::XComponentContext >& xContext )
{
    sal_Int32 nCooSysIndex = bMainAxis ? 0 : 1;
    uno::Reference< XBoundedCoordinateSystem > xCooSys = MeterHelper::getBoundedCoordinateSystemByIndex( xDiagram, nCooSysIndex  );
    if(!xCooSys.is())
    {
        //create a new CooSys by cloning it from another exsisting one
        uno::Reference< XBoundedCoordinateSystem > xOtherCooSys
            = MeterHelper::getBoundedCoordinateSystemByIndex( xDiagram, nCooSysIndex-1 );
        if(!xOtherCooSys.is()) //@todo? ...
            return;

        uno::Reference< util::XCloneable > xCloneable( xOtherCooSys,uno::UNO_QUERY);
        if( xCloneable.is() )
            xCooSys = uno::Reference< XBoundedCoordinateSystem >( xCloneable->createClone(), uno::UNO_QUERY );

        uno::Reference< XBoundedCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY );
        if( xCooSysCnt.is() && xCooSys.is() )
            xCooSysCnt->addCoordinateSystem( xCooSys );
    }
    // create axis
    MeterHelper::createAxis( nDimensionIndex, xCooSys, uno::Reference< XAxisContainer >( xDiagram, uno::UNO_QUERY ), xContext );
}

void createGrid( sal_Int32 nDimensionIndex, sal_Int32 nCooSysIndex, bool bMainGrid
                , const uno::Reference< XDiagram >& xDiagram
                , const uno::Reference< uno::XComponentContext >& xContext )
{
    uno::Reference< XBoundedCoordinateSystem > xCooSys = MeterHelper::getBoundedCoordinateSystemByIndex( xDiagram, nCooSysIndex );
    if(!xCooSys.is())
    {
        //@todo create initial coosys if none exsists (but this state is not likly to occur)
        return;
    }
    // create grid
    uno::Reference< XGrid > xGrid;
    OSL_ENSURE( nDimensionIndex <= 3, "Check Code for more than 3 dimensions" );

    ::rtl::OUStringBuffer aId( MeterHelper::makeGridIdentifier( nDimensionIndex, nCooSysIndex, bMainGrid ) );

    ContextHelper::tContextEntryMapType aContextValues(
        ContextHelper::MakeContextEntryMap(
        C2U( "Identifier" ), uno::makeAny( aId.makeStringAndClear()) ));

    xGrid.set( xContext->getServiceManager()->createInstanceWithContext(
            C2U( "drafts.com.sun.star.chart2.Grid" ),
            ContextHelper::createContext( aContextValues, xContext ) )
            , uno::UNO_QUERY );
    OSL_ASSERT( xGrid.is());
    if( xGrid.is())
    {
        xGrid->attachCoordinateSystem( xCooSys, nDimensionIndex );
        uno::Reference< XGridContainer > xGridContainer( xDiagram, uno::UNO_QUERY );
        xGridContainer->addGrid( xGrid );
    }
}

bool changeExistingAxes( const uno::Reference< XDiagram >& xDiagram
                        , const uno::Sequence< sal_Bool >& rOldExistenceList
                        , const uno::Sequence< sal_Bool >& rNewExistenceList
                        , const uno::Reference< uno::XComponentContext >& xContext )
{
    bool bChanged = false;
    for(sal_Int32 nN=0;nN<6;nN++)
    {
        if(rOldExistenceList[nN]!=rNewExistenceList[nN])
        {
            bChanged = true;
            if(rNewExistenceList[nN]) //add axis
            {
                createAxis( nN%3, nN<3, xDiagram, xContext );
            }
            else //remove axis
            {
                uno::Reference< XAxisContainer > xAxisContainer( xDiagram, uno::UNO_QUERY );
                if(xAxisContainer.is())
                {
                    uno::Reference< XAxis > xAxis = MeterHelper::getAxis( nN%3, nN<3, xDiagram );
                    xAxisContainer->removeAxis(xAxis);
                }
            }
        }
    }
    return bChanged;
}

bool changeExistingGrids( const uno::Reference< XDiagram >& xDiagram
                        , const uno::Sequence< sal_Bool >& rOldExistenceList
                        , const uno::Sequence< sal_Bool >& rNewExistenceList
                        , const uno::Reference< uno::XComponentContext >& xContext )
{
    bool bChanged = false;
    for(sal_Int32 nN=0;nN<6;nN++)
    {
        if(rOldExistenceList[nN]!=rNewExistenceList[nN])
        {
            bChanged = true;
            if(rNewExistenceList[nN]) //add grid
            {
                createGrid( nN%3, 0, nN<3, xDiagram, xContext );
            }
            else //remove grid
            {
                uno::Reference< XGridContainer > xGridContainer( xDiagram, uno::UNO_QUERY );
                if(xGridContainer.is())
                {
                    uno::Reference< XGrid > xGrid = MeterHelper::getGrid( nN%3, nN<3, xDiagram );
                    xGridContainer->removeGrid(xGrid);
                }
            }
        }
    }
    return bChanged;
}

//-----------------------------------------------------------------------------
//status of titles
void fillInsertTitleDialogDataFromModel( InsertTitleDialogData& rDialogData
        , const uno::Reference< frame::XModel>& xChartModel )
{
    uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram(xChartModel);

    //get possibilities
    uno::Sequence< sal_Bool > aMeterPossibilityList;
    MeterHelper::getMeterPossibilities( aMeterPossibilityList, xDiagram );
    rDialogData.aPossibilityList[2]=aMeterPossibilityList[0];//x axis title
    rDialogData.aPossibilityList[3]=aMeterPossibilityList[1];//y axis title
    rDialogData.aPossibilityList[4]=aMeterPossibilityList[2];//z axis title

    //find out which title exsist and get their text
    //main title:
    for( sal_Int32 nTitleIndex = static_cast< sal_Int32 >( TitleHelper::TITLE_BEGIN);
         nTitleIndex < static_cast< sal_Int32 >( TitleHelper::TITLE_END );
         nTitleIndex++)
    {
        uno::Reference< XTitle > xTitle =  TitleHelper::getTitle(
            static_cast< TitleHelper::eTitleType >( nTitleIndex ), xChartModel );
        rDialogData.aExistenceList[nTitleIndex] = xTitle.is();
        rDialogData.aTextList[nTitleIndex]=TitleHelper::getCompleteString( xTitle );
    }
}

//-----------------------------------------------------------------------------
//change existing titles

bool changeExistingTitles( const uno::Reference< frame::XModel >& xModel
                        , const InsertTitleDialogData& rOldState
                        , const InsertTitleDialogData& rNewState
                        , const uno::Reference< uno::XComponentContext >& xContext )
{
    bool bChanged = false;
    for( sal_Int32 nN = static_cast< sal_Int32 >( TitleHelper::TITLE_BEGIN );
         nN < static_cast< sal_Int32 >( TitleHelper::TITLE_END );
         nN++)
    {
        if( rOldState.aExistenceList[nN] != rNewState.aExistenceList[nN] )
        {
            if(rNewState.aExistenceList[nN])
            {
                TitleHelper::createTitle(
                    static_cast< TitleHelper::eTitleType >( nN ), rNewState.aTextList[nN], xModel, xContext );
                bChanged = true;
            }
            else
            {
                TitleHelper::removeTitle( static_cast< TitleHelper::eTitleType >( nN ), xModel );
                bChanged = true;
            }
        }
        else if( rOldState.aTextList[nN] != rNewState.aTextList[nN] )
        {
            //change content
            uno::Reference< XTitle > xTitle(
                TitleHelper::getTitle( static_cast< TitleHelper::eTitleType >( nN ), xModel ) );
            if(xTitle.is())
            {
                TitleHelper::setCompleteString( rNewState.aTextList[nN], xTitle, xContext );
                bChanged = true;
            }
        }
    }
    return bChanged;
}

}// anonymous namespace

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void SAL_CALL ChartController::executeDispatch_InsertAxis()
{
    try
    {
        Window* pParent( NULL );
        InsertMeterDialogData aDialogInput;
        uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram(m_aModel->getModel());
        getExistingAxes( aDialogInput.aExistenceList, xDiagram );
        MeterHelper::getMeterPossibilities( aDialogInput.aPossibilityList, xDiagram );

        SchAxisDlg aDlg( pParent, aDialogInput );
        if( aDlg.Execute() == RET_OK )
        {
            InsertMeterDialogData aDialogOutput;
            aDlg.getResult( aDialogOutput );
            bool bChanged = changeExistingAxes( xDiagram
                , aDialogInput.aExistenceList, aDialogOutput.aExistenceList, m_xCC );
            if(bChanged)
                impl_rebuildView();
        }
    }
    catch( uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void SAL_CALL ChartController::executeDispatch_InsertGrid()
{
    try
    {
        Window* pParent( NULL );
        InsertMeterDialogData aDialogInput;
        uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram(m_aModel->getModel());
        getExistingGrids( aDialogInput.aExistenceList, xDiagram );
        MeterHelper::getMeterPossibilities( aDialogInput.aPossibilityList, xDiagram, false );

        SchGridDlg aDlg( pParent, aDialogInput );//aItemSet, b3D, bNet, bSecondaryX, bSecondaryY );
        if( aDlg.Execute() == RET_OK )
        {
            InsertMeterDialogData aDialogOutput;
            aDlg.getResult( aDialogOutput );
            bool bChanged = changeExistingGrids( xDiagram
                , aDialogInput.aExistenceList, aDialogOutput.aExistenceList, m_xCC );
            if(bChanged)
                impl_rebuildView();
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
    try
    {
        Window* pParent( NULL );
        InsertTitleDialogData aDialogInput;
        fillInsertTitleDialogDataFromModel( aDialogInput, m_aModel->getModel() );

        SchTitleDlg aDlg( pParent, aDialogInput );
        if( aDlg.Execute() == RET_OK )
        {
            InsertTitleDialogData aDialogOutput;
            aDlg.getResult( aDialogOutput );
            bool bChanged = changeExistingTitles(
                m_aModel->getModel(), aDialogInput, aDialogOutput, m_xCC );
            if(bChanged)
                impl_rebuildView();
        }
    }
    catch( uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void SAL_CALL ChartController::executeDispatch_InsertLegend()
{
    bool bChanged = false;
    try
    {
        uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram(m_aModel->getModel());
        uno::Reference< beans::XPropertySet > xProp( xDiagram->getLegend(), uno::UNO_QUERY );
        //todo: If there is no legend, the dialog must be opened to add one
        if( ! xProp.is())
            return;
        wrapper::LegendItemConverter aItemConverter(
            xProp, m_pDrawModelWrapper->GetItemPool(), m_pDrawModelWrapper->getSdrModel() );
        SfxItemSet aItemSet = aItemConverter.CreateEmptyItemSet();
        aItemConverter.FillItemSet( aItemSet );

        //prepare and open dialog
        Window* pParent( NULL );
        SchLegendDlg aDlg( pParent, aItemSet);
        if( aDlg.Execute() == RET_OK )
        {
            SfxItemSet aOutItemSet = aItemConverter.CreateEmptyItemSet();
            aDlg.GetAttr( aOutItemSet );

            bChanged = aItemConverter.ApplyItemSet( aOutItemSet );//model should be changed now
        }
    }
    catch( uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
    //make sure that all objects using  m_pDrawModelWrapper or m_pChartView are already deleted
    if(bChanged) try
    {
        impl_rebuildView();
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
    bool bChanged = false;
    try
    {
        wrapper::AllDataLabelItemConverter aItemConverter(
            m_aModel->getModel(),
            m_pDrawModelWrapper->GetItemPool(),
            m_pDrawModelWrapper->getSdrModel());
        SfxItemSet aItemSet = aItemConverter.CreateEmptyItemSet();
        aItemConverter.FillItemSet( aItemSet );

        //prepare and open dialog
        Window* pParent( NULL );
        SchDataDescrDlg aDlg( pParent, aItemSet);
        if( aDlg.Execute() == RET_OK )
        {
            SfxItemSet aOutItemSet = aItemConverter.CreateEmptyItemSet();
            aDlg.GetAttr( aOutItemSet );
            bChanged = aItemConverter.ApplyItemSet( aOutItemSet );//model should be changed now
        }
    }
    catch( uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
    //make sure that all objects using  m_pDrawModelWrapper or m_pChartView are already deleted
    if(bChanged) try
    {
        impl_rebuildView();
    }
    catch( uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

void SAL_CALL ChartController::executeDispatch_InsertStatistic()
{
    try
    {
        bool bChanged = false;

        uno::Reference< beans::XPropertySet > xProp=NULL;
        //@todo use correct ItemConverter if available
//         wrapper::ChartTypeItemConverter aItemConverter( NULL, xProp, m_pDrawModelWrapper->GetItemPool() );
//         SfxItemSet aItemSet = aItemConverter.CreateEmptyItemSet();
        SfxItemSet aItemSet( m_pDrawModelWrapper->GetItemPool(), 1, 2 );
        //aItemConverter.FillItemSet( aItemSet );

        //prepare and open dialog
        Window* pParent( NULL );
        SchDataStatisticsDlg aDlg( pParent, aItemSet);
        if( aDlg.Execute() == RET_OK )
        {
//             SfxItemSet aOutItemSet = aItemConverter.CreateEmptyItemSet();
            SfxItemSet aOutItemSet( m_pDrawModelWrapper->GetItemPool(), 1, 2 );
            aDlg.GetAttr( aOutItemSet );

//             bChanged = aItemConverter.ApplyItemSet( aOutItemSet );//model should be changed now
        }
    }
    catch( uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
